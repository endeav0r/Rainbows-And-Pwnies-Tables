env = Environment()

env.AppendUnique(CFLAGS=['-O3', '-Wall', '-pthread', '-Wno-unused-function', '-fno-strict-aliasing'])
env.AppendUnique(LINKFLAGS=['-pthread', '-lm'])

src_c     = ['md4', 'md5', 'nt', 'plaintext', 'chain', 'hash']

sources_c =  map(lambda x: 'src/' + x + '.c', src_c)

env.Program(target="rtgen",
            source=sources_c + ['src/rtgen.c'],
           )

env.Program(target="rtcrack",
            source=sources_c + ['src/rtcrack.c'],
           )

env.Program(target="rtprint",
            source=sources_c + ['src/rtprint.c'],
           )

env.Program(target="rtinfo",
            source=sources_c + ['src/rtinfo.c'],
           )

env.Program(target="rtextend",
            source=sources_c + ['src/rtextend.c'],
           )

env.Program(target="rtmerge",
            source=sources_c + ['src/rtmerge.c'],
           )
