# My-first-repo
Terminal 3D .obj viewer in C and Python (custom ear clipping triangulation, z buffering, live controls). Learning project from Oct 2026, with very little AI use. Still has known issues to fix.

It's my first GitHub project, about 3 months into coding, so it's rough around the edges, but it works and I'm pretty happy with it.

> **How it was made:** Built in about 10 days, with very little AI help. I only used it for syntax and for learning new things I hadn't touched before, mainly terminal canonical mode and `expect`. The logic and the rest of the code are my own, apart from Newell's method for the face normal (see the note in `v2_triangulate.py`), which I found online.

## What it does

1. **Parses** an `.obj` file (vertices and faces) into a simple text format.
2. **Triangulates** every face with an ear clipping approach(not exactly, but essentially the same) , so polygons with 4, 5, 6 or more sides get split into triangles. It uses Newell's method to get each face's normal and checks that no other vertex sits inside a candidate ear.
3. **Renders** the triangles in C: Bresenham line drawing, a z buffer for hidden edges, and simple lighting where brightness depends on how much each face points toward the viewer. The output is drawn with ANSI color blocks.
4. **Lets you interact** with the shape live. The renderer puts the terminal out of canonical mode (no waiting for Enter) with echo off, so each keypress takes effect immediately.

## Project layout

```
final/
├── RENDER.py          # entry point: reads .obj from stdin, writes parsed.txt, runs the triangulator
├── v2_triangulate.py  # triangulates faces, writes triangulated.txt, launches the renderer via expect
├── v3_renderer.c      # the renderer (C)
├── v3_renderer        # compiled binary (Linux x86-64)
├── parsed.txt         # intermediate output of RENDER.py (regenerated each run)
├── triangulated.txt   # intermediate output of v2_triangulate.py (regenerated each run)
└── models/            # a bunch of sample .obj files
```

The data flows like this:

```
model.obj  ->  RENDER.py  ->  parsed.txt  ->  v2_triangulate.py  ->  triangulated.txt  ->  v3_renderer
```

## Requirements

- Linux (or something similar with a Unix-style terminal; it uses `termios`)
- Python 3
- `gcc` (only needed if you want to recompile the renderer)
- [`expect`](https://core.tcl-lang.org/expect/index), which is how the triangulator hands the data to the renderer and then gives the keyboard back to it
- A terminal with true-colour support, and a big window (see the note below)
- A perfectly square terminal font. It runs without it, however it results in a vertically stretched render, since most fonts are taller than they are wide. unscii-8 is a perfectly square terminal font.

## Setup

Compile the renderer:

```bash
gcc v3_renderer.c -o v3_renderer -lm
```

## Usage

From inside the project folder, pipe an `.obj` file into `RENDER.py`:

```bash
python3 RENDER.py < models/shark.obj
```

Swap `shark.obj` for any file in `models/`.

### Controls

| Key | Action |
| --- | --- |
| `w` / `s` | rotate around the X axis |
| `a` / `d` | rotate around the Y axis |
| `q` / `e` | rotate around the Z axis |
| `+` / `-` | zoom in / out |
| `i` / `k` | move up / down |
| `j` / `l` | move left / right |
| `x` | quit |

## Things to know

- **The canvas is 150 columns by 100 rows.** Your terminal needs to be at least that big, so zoom the font way out (e.g. `Ctrl` + `-`) or you'll see a mess.
- **Model size matters.** `shark.obj` has coordinates in the 0 to 100 range, which fits the canvas nicely. Most of the other models are unit-sized (roughly -1 to 1), so they start out tiny and may be partly off-screen. Use `+` to zoom in and `i`/`j`/`k`/`l` to bring them into view.
- **Limits:** up to 100 triangles total, up to 100 face edges total, and at most 30 vertices per face. The renderer prints an error if a model goes over. This is the next main improvement I'd work on(its lingered because I got carried away making the rest of the code).
- There is quite a major issue where edges of the rendered objects don't look right; They have small gaps in them. That's also one of the major fixes to be done, likely using polygon offset.
- `parsed.txt` and `triangulated.txt` get overwritten every run.
- `v2_triangulate.py` opens `/dev/tty` to give the keyboard to the renderer, so run it from a real terminal rather than a pipe or an IDE output panel.

## Sample models

`cube`, `dodecahedron`, `icosahedron`, `heart`, `shark`, `c_shark`, `rocket`, `starfighter`, `gem`, `crystals`, `jack`, `kis_cube`, `lightning_bolt`, `prism`, `hexagonal_prism`, `five_point_star_prism`, `stella_octangula`, `cube_octahedron_compound`, `truncated_octahedron`, `twisted_tower`, `square_frame`, `triangle_frame`.

## Ideas for later

- Collapse the repeated rotate/zoom/move code in the main loop into functions
- Auto-scale and auto-centre models so every `.obj` fits the canvas
- Raise the triangle and edge limits, and fix the issue with the objects' edges.
- A single script or Makefile instead of the Python-to-expect-to-C chain
