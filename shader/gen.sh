#! /bin/sh
xxd -i shader/cell.vert > shader/cell.vert.h
xxd -i shader/cell.frag > shader/cell.frag.h
xxd -i shader/normal.frag > shader/normal.frag.h
xxd -i shader/normal.vert > shader/normal.vert.h
xxd -i shader/color.vert > shader/color.vert.h
xxd -i shader/color.frag > shader/color.frag.h
xxd -i shader/texture.vert > shader/texture.vert.h
xxd -i shader/texture.frag > shader/texture.frag.h
xxd -i shader/color_normal_texture.vert > shader/color_normal_texture.vert.h
xxd -i shader/color_normal_texture.frag > shader/color_normal_texture.frag.h
