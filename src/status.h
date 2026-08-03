#ifndef STATUS_H
#define STATUS_H

#define STATUS_OK 0
#define EIO 1 //ERROR: IO
#define EINVARG 2 //ERROR: Invalid Argument
#define ENOMEM 3 //ERROR: no memory (out of memory)
#define EBADPATH 4 //ERROR: bad path (for filesystem)
#define EFSNOTUS 5 // ERROR: not our filesystem
#define ERDONLY 6 // ERROR: file mode read only
#define EUNIMP 7 // ERROR: unimplemented

#endif