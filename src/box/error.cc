/*
 * Copyright 2010-2016, Tarantool AUTHORS, please see AUTHORS file.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "error.h"
#include <stdio.h>

#include "event.h"
#include "fiber.h"
#include "func_adapter.h"
#include "rmean.h"
#include "trigger.h"
#include "vclock/vclock.h"
#include "schema.h"
#include "ssl_error.h"
#include "tuple.h"

/* {{{ public API */

const char *
box_error_type(const box_error_t *e)
{
	return e->type->name;
}

uint32_t
box_error_code(const box_error_t *e)
{
	return ClientError::get_errcode(e);
}

const char *
box_error_message(const box_error_t *error)
{
	return error->errmsg;
}

box_error_t *
box_error_last(void)
{
	return diag_last_error(&fiber()->diag);
}

void
box_error_clear(void)
{
	diag_clear(&fiber()->diag);
}

int
box_error_set(const char *file, unsigned line, uint32_t code,
		const char *fmt, ...)
{
	struct error *e = BuildClientError(file, line, ER_UNKNOWN);
	ClientError *client_error = type_cast(ClientError, e);
	client_error->code = code;
	va_list ap;
	va_start(ap, fmt);
	error_vformat_msg(e, fmt, ap);
	va_end(ap);
	diag_set_error(&fiber()->diag, e);
	return -1;
}

static struct error *
box_error_new_va(const char *file, unsigned line, uint32_t code,
		 const char *custom_type, const char *fmt, va_list ap)
{
	struct error *e;
	if (custom_type != NULL) {
		e = BuildCustomError(file, line, custom_type, code);
	} else if (code == ER_ILLEGAL_PARAMS) {
		e = BuildIllegalParams(file, line, "");
	} else {
		e = BuildClientError(file, line, ER_UNKNOWN);
		ClientError *client_error = type_cast(ClientError, e);
		client_error->code = code;
	}
	error_vformat_msg(e, fmt, ap);
	return e;
}

struct error *
box_error_new(const char *file, unsigned line, uint32_t code,
	      const char *custom_type, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	struct error *e = box_error_new_va(file, line, code, custom_type,
					   fmt, ap);
	va_end(ap);
	return e;
}

int
box_error_add(const char *file, unsigned line, uint32_t code,
	      const char *custom_type, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	struct error *e = box_error_new_va(file, line, code, custom_type,
					   fmt, ap);
	va_end(ap);

	struct diag *d = &fiber()->diag;
	if (diag_is_empty(d))
		diag_set_error(d, e);
	else
		diag_add_error(d, e);
	return -1;
}

/* }}} */

const char *
box_error_custom_type(const struct error *e)
{
	CustomError *custom_error = type_cast(CustomError, e);
	if (custom_error)
		return custom_error->custom_type();

	return NULL;
}

struct rmean *rmean_error = NULL;

const char *rmean_error_strings[RMEAN_ERROR_LAST] = {
	"ERROR"
};

/** Format client error with arguments in `ap` according to error format. */
static void
client_error_create(struct error *e, va_list ap)
{
	const struct errcode_record *r = tnt_errcode_record(e->code);
	va_list ap_copy;
	va_copy(ap_copy, ap);
	error_vformat_msg(e, r->errdesc, ap_copy);
	va_end(ap_copy);
	for (int i = 0; i < r->errfields_count; i++) {
		const char *name = r->errfields[i].name;
		bool set_payload = name[0] != '\0';
		switch (r->errfields[i].type) {
		case ERRCODE_FIELD_TYPE_CHAR: {
			char buf[2] = {(char)va_arg(ap, int), '\0'};
			if (set_payload)
				error_set_str(e, name, buf);
			break;
		}
		case ERRCODE_FIELD_TYPE_INT: {
			int v = va_arg(ap, int);
			if (set_payload)
				error_set_int(e, name, v);
			break;
		}
		case ERRCODE_FIELD_TYPE_UINT: {
			unsigned v = va_arg(ap, unsigned);
			if (set_payload)
				error_set_uint(e, name, v);
			break;
		}
		case ERRCODE_FIELD_TYPE_LONG: {
			long v = va_arg(ap, long);
			if (set_payload)
				error_set_int(e, name, v);
			break;
		}
		case ERRCODE_FIELD_TYPE_ULONG: {
			unsigned long v = va_arg(ap, unsigned long);
			if (set_payload)
				error_set_uint(e, name, v);
			break;
		}
		case ERRCODE_FIELD_TYPE_LLONG: {
			long long v = va_arg(ap, long long);
			if (set_payload)
				error_set_int(e, name, v);
			break;
		}
		case ERRCODE_FIELD_TYPE_ULLONG: {
			unsigned long long v = va_arg(ap, unsigned long long);
			if (set_payload)
				error_set_uint(e, name, v);
			break;
		}
		case ERRCODE_FIELD_TYPE_STRING: {
			const char *s = va_arg(ap, const char *);
			if (set_payload && s != NULL)
				error_set_str(e, name, s);
			break;
		}
		case ERRCODE_FIELD_TYPE_MSGPACK: {
			const char *mp = va_arg(ap, const char *);
			const char *mp_end = mp;
			assert(set_payload);
			if (mp != NULL) {
				mp_next(&mp_end);
				error_set_mp(e, name, mp, mp_end - mp);
			}
			break;
		}
		case ERRCODE_FIELD_TYPE_TUPLE: {
			struct tuple *tuple = va_arg(ap, struct tuple *);
			assert(set_payload);
			if (tuple != NULL)
				error_set_mp(e, name, tuple_data(tuple),
					     tuple_bsize(tuple));
			break;
		}
		default:
			assert(false);
		}
	}
	assert(strncmp("ER_", r->errstr, 3) == 0);
	error_set_str(e, "name", r->errstr + 3);
}

const struct type_info type_ClientError =
	make_type("ClientError", &type_Exception);

ClientError::ClientError(const type_info *type, const char *file, unsigned line,
			 uint32_t errcode)
	:Exception(type, file, line)
{
	code = errcode;
	if (rmean_error)
		rmean_collect(rmean_error, RMEAN_ERROR, 1);
}

ClientError::ClientError(const char *file, unsigned line,
			 uint32_t errcode, ...)
	:ClientError(&type_ClientError, file, line, errcode)
{
	va_list ap;
	va_start(ap, errcode);
	client_error_create(this, ap);
	va_end(ap);
}

struct error *
BuildClientError(const char *file, unsigned line, uint32_t errcode, ...)
{
	assert(errcode != ER_ILLEGAL_PARAMS); /* use IllegalParams */
	assert(errcode != ER_MEMORY_ISSUE); /* use OutOfMemory */
	assert(errcode != ER_SYSTEM); /* use SystemError */
	assert(errcode != ER_SSL); /* use SSLError */
	assert(errcode != ER_XLOG_GAP); /* use XlogGapError */
	assert(errcode != ER_ACCESS_DENIED); /* use AccessDeniedError */
	ClientError *e = new ClientError(file, line, ER_UNKNOWN);
	va_list ap;
	va_start(ap, errcode);
	e->code = errcode;
	client_error_create(e, ap);
	va_end(ap);
	return e;
}

void
ClientError::log() const
{
	say_file_line(S_ERROR, file, line, errmsg, "%s", tnt_errcode_str(code));
}


uint32_t
ClientError::get_errcode(const struct error *e)
{
	ClientError *client_error = type_cast(ClientError, e);
	if (client_error)
		return client_error->errcode();
	if (type_cast(IllegalParams, e))
		return ER_ILLEGAL_PARAMS;
	if (type_cast(OutOfMemory, e))
		return ER_MEMORY_ISSUE;
	if (type_cast(SystemError, e))
		return ER_SYSTEM;
	if (type_cast(SSLError, e))
		return ER_SSL;
	if (type_cast(CollationError, e))
		return ER_CANT_CREATE_COLLATION;
	if (type_cast(XlogGapError, e))
		return ER_XLOG_GAP;
	return ER_PROC_LUA;
}

const struct type_info type_XlogError = make_type("XlogError", &type_Exception);

struct error *
BuildXlogError(const char *file, unsigned line, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	XlogError *e = new XlogError(file, line, format, ap);
	va_end(ap);
	return e;
}

const struct type_info type_XlogGapError =
	make_type("XlogGapError", &type_XlogError);

XlogGapError::XlogGapError(const char *file, unsigned line,
			   const struct vclock *from, const struct vclock *to)
		: XlogError(&type_XlogGapError, file, line)
{
	const char *s_from = vclock_to_string(from);
	const char *s_to = vclock_to_string(to);
	error_format_msg(this,
			 "Missing .xlog file between LSN %lld %s and %lld %s",
			 (long long)vclock_sum(from), s_from ? s_from : "",
			 (long long)vclock_sum(to), s_to ? s_to : "");
}

struct error *
BuildXlogGapError(const char *file, unsigned line,
		  const struct vclock *from, const struct vclock *to)
{
	return new XlogGapError(file, line, from, to);
}

struct rlist on_access_denied = RLIST_HEAD_INITIALIZER(on_access_denied);

const struct type_info type_AccessDeniedError =
	make_type("AccessDeniedError", &type_ClientError);

AccessDeniedError::AccessDeniedError(const char *file, unsigned int line,
				     const char *access_type,
				     const char *object_type,
				     const char *object_name,
				     const char *user_name,
				     bool run_triggers)
	:ClientError(&type_AccessDeniedError, file, line, ER_ACCESS_DENIED)
{
	error_format_msg(this, tnt_errcode_desc(code),
			 access_type, object_type, object_name, user_name);
	struct on_access_denied_ctx trigger_ctx =
		{access_type, object_type, object_name};
	/*
	 * Don't run the triggers when create after marshaling
	 * through network.
	 */
	if (run_triggers) {
		if (trigger_run(&on_access_denied, &trigger_ctx) != 0)
			diag_log();
	}
	error_set_str(this, "object_type", object_type);
	error_set_str(this, "object_name", object_name);
	error_set_str(this, "access_type", access_type);
	error_set_str(this, "user", user_name);
}

struct error *
BuildAccessDeniedError(const char *file, unsigned int line,
		       const char *access_type, const char *object_type,
		       const char *object_name,
		       const char *user_name)
{
	return new AccessDeniedError(file, line, access_type,
				     object_type, object_name,
				     user_name);
}

const struct type_info type_CustomError =
	make_type("CustomError", &type_ClientError);

CustomError::CustomError(const char *file, unsigned int line,
			 const char *custom_type, uint32_t errcode)
	:ClientError(&type_CustomError, file, line, errcode)
{
	error_set_str(this, "custom_type", custom_type);
}

void
CustomError::log() const
{
	say_file_line(S_ERROR, file, line, errmsg,
		      "Custom type %s", custom_type());
}

struct error *
BuildCustomError(const char *file, unsigned int line, const char *custom_type,
		 uint32_t errcode)
{
	return new CustomError(file, line, custom_type, errcode);
}
