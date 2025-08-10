#ifndef _CONFIG_H
#define _CONFIG_H

#ifndef _DEFAULT_ASSETS_PATH
#error "DEFAULT_ASSETS_PATH undefined"
#endif
#ifndef _CHUNK_SIZE
#error "CHUNK_SIZE undefined"
#endif

#define DEFAULT_ASSETS_PATH _DEFAULT_ASSETS_PATH
#define CHUNK_SIZE _CHUNK_SIZE

#endif