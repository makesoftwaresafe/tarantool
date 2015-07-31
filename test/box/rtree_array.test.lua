s = box.schema.space.create('spatial')
_ = s:create_index('primary')
spatial = s:create_index('spatial', { type = 'rtree', unique = false, parts = {2, 'array'}})

spatial.type

s:insert{1,{0.0,0.0}}
s:insert{2,{0.0,10.0}}
s:insert{3,{0.0,50.0}}
s:insert{4,{10.0,0.0}}
s:insert{5,{50.0,0.0}}
s:insert{6,{10.0,10.0}}
s:insert{7,{10.0,50.0}}
s:insert{8,{50.0,10.0}}
s:insert{9,{50.0,50.0}}

-- select all records
s.index.spatial:select({iterator = 'ALL'})
-- select records belonging to rectangle (0,0,10,10)
s.index.spatial:select({0.0,0.0,10.0,10.0}, {iterator = 'LE'})
-- select records with coordinates (10,10)
s.index.spatial:select({10.0,10.0}, {iterator = 'EQ'})
-- select neighbors of point (5,5)
s.index.spatial:select({5.0,5.0}, {iterator = 'NEIGHBOR'})

s:drop()

s = box.schema.space.create('spatial')
_ = s:create_index('primary')
spatial = s:create_index('spatial', { type = 'rtree', unique = false, parts = {2, 'array'}, dimension = 8})

spatial.type

s:insert{ 1,{0, 0, 0, 0, 0, 0, 0, 0}}
s:insert{ 2,{10, 0, 0, 0, 0, 0, 0, 0}}
s:insert{ 3,{0, 10, 0, 0, 0, 0, 0, 0}}
s:insert{ 4,{0, 0, 10, 0, 0, 0, 0, 0}}
s:insert{ 5,{0, 0, 0, 10, 0, 0, 0, 0}}
s:insert{ 6,{0, 0, 0, 0, 10, 0, 0, 0}}
s:insert{ 7,{0, 0, 0, 0, 0, 10, 0, 0}}
s:insert{ 8,{0, 0, 0, 0, 0, 0, 10, 0}}
s:insert{ 9,{0, 0, 0, 0, 0, 0, 0, 10}}
s:insert{10,{50, 0, 0, 0, 0, 0, 0, 0}}
s:insert{11,{0, 50, 0, 0, 0, 0, 0, 0}}
s:insert{12,{0, 0, 50, 0, 0, 0, 0, 0}}
s:insert{13,{0, 0, 0, 50, 0, 0, 0, 0}}
s:insert{14,{0, 0, 0, 0, 50, 0, 0, 0}}
s:insert{15,{0, 0, 0, 0, 0, 50, 0, 0}}
s:insert{16,{0, 0, 0, 0, 0, 0, 50, 0}}
s:insert{17,{0, 0, 0, 0, 0, 0, 0, 50}}
s:insert{18,{10, 10, 10, 10, 10, 10, 10, 10}}
s:insert{19,{10, 50, 10, 50, 10, 50, 10, 50}}
s:insert{20,{0, 10, 50, 0, 10, 50, 0, 10}}

p0 = {0, 0, 0, 0, 0, 0, 0, 0}
p5 = {5, 5, 5, 5, 5, 5, 5, 5}
p10 = {10, 10, 10, 10, 10, 10, 10, 10 }
rt0_10 = {0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 10, 10, 10, 10, 10 }

-- select all records
s.index.spatial:select({iterator = 'ALL'})
-- select records belonging to rectangle (0,0,..10,10,..)
s.index.spatial:select(rt0_10, {iterator = 'LE'})
-- select records with coordinates (10,10)
s.index.spatial:select(p10, {iterator = 'EQ'})
-- select neighbors of point (5,5)
s.index.spatial:select(p5, {iterator = 'NEIGHBOR'})

s:drop()
