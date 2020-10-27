all : \
    ../dist/tetris.pnct \
    ../dist/tetris.scene \
#    ../dist/tetris.w \

../dist/tetris.scene : tetris.blend export-scene.py
    "C:\Program Files\Blender Foundation\Blender 2.90\blender.exe" --background --python export-scene.py -- "tetris.blend:Collection" "../dist/tetris.scene"

../dist/tetris.pnct : tetris.blend export-meshes.py
    "C:\Program Files\Blender Foundation\Blender 2.90\blender.exe" --background --python export-meshes.py -- "tetris.blend:Collection" "../dist/tetris.pnct" 

#../dist/tetris.w : tetris.blend export-walkmeshes.py
#    "C:\Program Files\Blender Foundation\Blender 2.90\blender.exe" --background --python export-walkmeshes.py -- "tetris.blend:Collection" "../dist/tetris.w" 