#~25/10/26

import sys
import subprocess

f_count = 0
vertices = []
count = 0
face_vertices_list = []
compatible_output = []
face_vertices = 0
templist = []

for line in sys.stdin:
    parts = []
    parts = line.split()

    if len(parts) == 0:
        continue
    elif parts[0] == '#':
        continue
    elif parts[0] == 'v':
        vertices.append([float(parts[1]), float(parts[2]), float(parts[3])])
        count += 1
    elif parts[0] == 'f':
        templist = []
        f_count += 1
        face_vertices = len(parts) - 1
        face_vertices_list.append(face_vertices)
        for i in range(face_vertices):
            index = int(parts[i+1]) - 1
            templist.append(vertices[index])

        compatible_output.append(templist) 

with open("parsed.txt", "w") as f:
    print(f_count, file=f)

    for i in range(f_count):
        print(face_vertices_list[i], file=f)
        for j in range(face_vertices_list[i]):
            print(compatible_output[i][j][0], compatible_output[i][j][1], compatible_output[i][j][2], file=f)

with open("parsed.txt") as f:
    result = subprocess.run([sys.executable, "v2_triangulate.py"], stdin=f)

print("exit code:", result.returncode)











    
