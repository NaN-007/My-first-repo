import subprocess

EPSILON = 1e-6

def dot_product(v1, v2):
    product = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2]
    return product

def cross_product(v1, v2, result):
    result[0] = v1[1]*v2[2] - v1[2]*v2[1]
    result[1] = v1[2]*v2[0] - v1[0]*v2[2]
    result[2] = v1[0]*v2[1] - v1[1]*v2[0]
    return None

#oriented normal method was mostly from online(Newell's method), rest of it is my own work
def oriented_normal(vertices_in_face, normal_vector):
    temporary_cross = [0, 0, 0]
    v1 = []
    v2 = []
    for i in range(len(vertices_in_face)):
        if i == len(vertices_in_face) - 1:
            v1 = vertices_in_face[i][:]
            v2 = vertices_in_face[0][:]
        else:
            v1 = vertices_in_face[i][:]
            v2 = vertices_in_face[i+1][:]

        cross_product(v1, v2, temporary_cross)

        normal_vector[0] += temporary_cross[0]
        normal_vector[1] += temporary_cross[1]
        normal_vector[2] += temporary_cross[2]

    return None    

def subtract_vectors(vector_a, vector_b):
    return [vector_a[0] - vector_b[0], vector_a[1] - vector_b[1], vector_a[2] - vector_b[2]]

def point_in_triangle(point, corner_a, corner_b, corner_c, face_normal):
    edge_cross = [0, 0, 0]
    triangle_edges = ((corner_a, corner_b), (corner_b, corner_c), (corner_c, corner_a))

    for edge_start, edge_end in triangle_edges:
        edge_vector = subtract_vectors(edge_end, edge_start)
        to_point_vector = subtract_vectors(point, edge_start)

        cross_product(edge_vector, to_point_vector, edge_cross)

        if dot_product(edge_cross, face_normal) < -EPSILON:
            return False

    return True

def triangulate(no_of_faces, faces_vertices_list, compatible_output):
    no_of_tris = 0

    for i in range(no_of_faces):
        applicable_lines = []
        normal_vector = [0, 0 ,0]
        temporary_normal = [0, 0, 0]
        #compatible_output = []
        vertices_in_face = faces_vertices_list[i][:]
        
        oriented_normal(vertices_in_face, normal_vector)

        for j in range(len(vertices_in_face)):
            if j == len(vertices_in_face) - 1:
                a = faces_vertices_list[i][:]
                b = a[j] + a[0]
            else:
                a = faces_vertices_list[i][:]
                b = a[j] + a[j+1]

            applicable_lines.append(b)

        while len(applicable_lines) != 2: 
            actual_index = 0
            index = actual_index - len(applicable_lines)

            while index != 0 and index != -1:
                line1 = applicable_lines[actual_index]
                line2 = applicable_lines[actual_index + 1]
                line3 = []

                #diag in poly test
                v1 = [line1[j+3] - line1[j] for j in range(len(line1)-3)]
                v2 = [line2[j+3] - line2[j] for j in range(len(line2)-3)]

                cross_product(v1, v2, temporary_normal)
                dot = dot_product(temporary_normal, normal_vector)

                if dot < EPSILON:   #covers both reflex and collinear
                    actual_index += 1
                    index = actual_index - len(applicable_lines)
                    continue
                elif dot >= EPSILON:
                    p = line1[:3]
                    q = line1[3:] # == line2[:3]
                    r = line2[3:]

                    line_count = len(applicable_lines)
                    skip_indices = (actual_index, actual_index + 1, (actual_index + 2) % line_count)
                    invalid = False

                    for k in range(line_count):
                        if k in skip_indices:
                            continue
                        if point_in_triangle(applicable_lines[k][:3], p, q, r, normal_vector):
                            invalid = True
                            break

                    if invalid == True:
                        actual_index += 1
                        index = actual_index - len(applicable_lines)
                        continue

                    line3 += line1[:3] + line2[3:]
                    applicable_lines[actual_index] = line3[:]
                    applicable_lines.pop(actual_index + 1)

                    p = line1[:3]
                    q = line1[3:] # == line2[:3]
                    r = line2[3:]
                    t = [p, q, r]
                    compatible_output.append(t)

                    actual_index += 1
                    index = actual_index - len(applicable_lines)
                    no_of_tris += 1

    return no_of_tris

#MAIN

#input
no_of_faces = int(input())
faces_vertices_list = []   #its [][][]
compatible_output = []     #its [][3][]
n_vertices_list = []

for i in range(no_of_faces):
    n_vertices = int(input())
    n_vertices_list.append(n_vertices)
    y = []
    for j in range(n_vertices):
        x = input().split()

        for k in range(len(x)):
            x[k] = float(x[k])

        y.append(x)

    faces_vertices_list.append(y)

#process
triangle_no = triangulate(no_of_faces, faces_vertices_list, compatible_output)

#output

#w clears old content, a appends
with open("triangulated.txt", "w") as f:
    print(triangle_no, file=f)

    for i in range(len(compatible_output)):
        for j in range(3):
            print(compatible_output[i][j][0], 
                  compatible_output[i][j][1], 
                  compatible_output[i][j][2], 
                  file=f)
        
        print("\n", file=f)

    print(no_of_faces, file=f)
    for i in range(no_of_faces):
        print(n_vertices_list[i], file=f)
        for j in range(n_vertices_list[i]):
            print(faces_vertices_list[i][j][0], faces_vertices_list[i][j][1], faces_vertices_list[i][j][2], file=f)

with open("triangulated.txt", "r") as f:
    file_content = f.read()

# 2. Escape double quotes or special characters for the shell string
escaped_content = file_content.replace('"', '\\"').replace('$', '\\$')

# 3. Create an expect command that:
#    - Spawns your C binary inside a fake terminal
#    - Sends the text file data instantly
#    - Interacts natively, passing control back to your keyboard
expect_script = f'''
spawn ./v3_renderer
send "{escaped_content}"
interact
'''

# 4. Execute the command via Python subprocess
with open("/dev/tty") as tty:
    subprocess.run(["expect", "-c", expect_script], stdin=tty)









        

