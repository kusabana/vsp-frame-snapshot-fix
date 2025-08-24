#include <cstdio>
#include <cstring>
#include <link.h>
#include <sys/mman.h>
#include <sys/stat.h>

namespace sym {
  auto resolve( void *handle, const char *name ) -> void * {
    const struct link_map *lm;
    void *result = nullptr;
    void *base = nullptr;
    FILE *f = nullptr;
    struct stat s;

    unsigned int i, symcount = 0;
    ElfW( Ehdr ) * e;
    ElfW( Shdr ) * shdr;
    ElfW( Sym ) * sym;
    char *strtab;

    if ( dlinfo( handle, RTLD_DI_LINKMAP, &lm ) != 0 )
      goto cleanup;

    f = fopen( lm->l_name, "r" );
    if ( !f )
      goto cleanup;

    if ( fstat( fileno( f ), &s ) != 0 )
      goto cleanup;

    base = mmap( nullptr, s.st_size, PROT_READ, MAP_PRIVATE, fileno( f ), 0 );
    if ( base == MAP_FAILED ) {
      base = nullptr;
      goto cleanup;
    }

    e = ( ElfW( Ehdr ) * ) base;
    shdr = ( ElfW( Shdr ) * ) ( ( char * ) base + e->e_shoff );
    sym = nullptr;
    strtab = nullptr;

    for ( i = 0; i < e->e_shnum; i++ ) {
      if ( shdr[ i ].sh_type == SHT_SYMTAB ) {
        sym = ( ElfW( Sym ) * ) ( ( char * ) base + shdr[ i ].sh_offset );
        symcount = shdr[ i ].sh_size / sizeof( ElfW( Sym ) );
        strtab = ( char * ) base + shdr[ shdr[ i ].sh_link ].sh_offset;

        break;
      }
    }

    if ( !strtab || !sym || !symcount )
      goto cleanup;

    for ( i = 0; i < symcount; i++ ) {
      if ( !strcmp( &strtab[ sym[ i ].st_name ], name ) ) {
        result = ( void * ) ( lm->l_addr + sym[ i ].st_value );
        break;
      }
    }

  cleanup:
    if ( base )
      munmap( base, s.st_size );
    if ( f )
      fclose( f );
    return result;
  }
} // namespace sym
