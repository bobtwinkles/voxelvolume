# So we need to generate 256 different cubes, which is not my idea of "fun" so... python time!
# source: http://citeseerx.ist.psu.edu/viewdoc/download;jsessionid=611EC2E9830C52108DB5D9AB4B76B776?doi=10.1.1.56.7139&rep=rep1&type=pdf
import json

goal_var = "goal"
datastore_var = "ds"
ds_fetch_var = "{datastore}->GetPoint({x}, {y}, {z})"
x_base = "x"
y_base = "y"
z_base = "z"
point_format = "x{0}y{1}z{2}"
lerp_format  = "lerp_{p1}_{p2}"

def count_bits(n):
    m = 0
    while n != 0:
        m += n & 1
        n >>= 1
    return m

def emit_code(s, increment_level=0, increment_first=False):
    global indent_level
    if increment_first:
        indent_level += increment_level
    print("  " * indent_level + s)
    if not(increment_first):
        indent_level += increment_level

def emit_vertex(x, y, z):
    emit_code("glVertex3f(x + {x}, y + {y}, x + {z});".format(x = x, y = y, z = z) )

def get_point(x, y, z):
    return point_format.format(*(x, y, z))

def offset_get(ox, oy, oz):
    return ds_fetch_var.format(datastore = datastore_var
                              ,x = x_base + " + " + str(ox)
                              ,y = y_base + " + " + str(oy)
                              ,z = z_base + " + " + str(oz))
def lerp_name(p1, p2):
    return lerp_format.format(p1=p1, p2=p2)

def generate_verts():
    emit_code("switch(state) {", 1)

    for i in range(0, 256):
        count = count_bits(i)
        emit_code("case {i}:".format(i=i), 1)
        if count == 1:
            corner = points[i]
            x, y, z = corner
            emit_vertex(0.5, corner[1], corner[2])
            emit_vertex(corner[0], 0.5, corner[2])
            emit_vertex(corner[0], corner[1], 0.5)
        emit_code("break;", -1)

    emit_code("}", -1, True)

def generate_lerps():
    global lerps
    for p in reverse_pairs:
        if len(p) != 2:
            continue
        print(p, reverse_pairs[p])
        for pair in reverse_pairs[p]:
            if pair[0] > pair[1]:
                pair = pair[1], pair[0]
            if pair[0] not in lerps:
                lerps[pair[0]] = {}
            if pair[1] not in lerps[pair[0]]:
                pl_1 = points[pair[0]]
                pl_2 = points[pair[1]]
                code = "float {lerp_name} = inverse_lerp({low}, {high}, {goal});".format(lerp_name=lerp_name(get_point(*pl_1), get_point(*pl_2)),
                                                                                         low  = offset_get(*pl_1),
                                                                                         high = offset_get(*pl_2),
                                                                                         goal = goal_var)
                lerps[pair[0]][pair[1]] = code
    print(json.dumps(lerps, indent=2))

lerps = {}

points = {1  : (0, 0, 0),       # b00000001
          2  : (1, 0, 0),       # b00000010
          4  : (0, 1, 0),       # b00000100
          8  : (1, 1, 0),       # b00001000
          16 : (0, 0, 1),       # b00010000
          32 : (1, 0, 1),       # b00100000
          64 : (0, 1, 1),       # b01000000
          128: (1, 1, 1)}       # b10000000

pairs = {}
reverse_pairs = {}

for i in range(0, 8):
    for j in range(i + 1, 8):
        p1i = 1 << i
        p2i = 1 << j
        p1 = points[p1i]
        p2 = points[p2i]
        same = ""
        if p1[0] == p2[0]:
            same += "x"
        if p1[1] == p2[1]:
            same += "y"
        if p1[2] == p2[2]:
            same += "z"
        if p1i not in pairs:
            pairs[p1i] = {}
        pairs[p1i][p2i] = same
        if p2i not in pairs:
            pairs[p2i] = {}
        pairs[p2i][p1i] = same
        if same not in reverse_pairs:
            reverse_pairs[same] = set([])
        reverse_pairs[same].add((p2i, p1i))
        reverse_pairs[same].add((p1i, p2i))

print(json.dumps(pairs, indent=4))

generate_lerps()
#generate_verts()
