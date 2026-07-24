# Tula Bytecode
The following document will outline the individual bytecode instructions used in
compiled tula (.tbin).

One stack "byte" is an 8-bit integer.
One stack "reference" (aka "ref") is a 64-bit integer, or equivilant to 8 bytes.
    NOTE: "arrayref" is also a reference

| Mnemonic | Opcode (hex) | Size | Other Bytes       | Stack Effect                                                    | Description                                                                        |
|----------|--------------|------|-------------------|-----------------------------------------------------------------|------------------------------------------------------------------------------------|
| nop      | 0x00         | 1    | N/A               | N/A                                                             | performs no operation                                                              |
| halt     | 0x01         | 1    | N/A               | N/A                                                             | halts the process                                                                  |
| reserved | 0x02         | -    | -                 | -                                                               | reserved for future use                                                            |
| pop      | 0x03         | 1    | N/A               | byte ->                                                         | discards the top byte (8-bits) of the stack                                        |
| pop2     | 0x04         | 1    | N/A               | byte2, byte1 ->                                                 | discards the top 2 bytes (16-bits) of the stack                                    |
| pop4     | 0x05         | 1    | N/A               | byte4, ..., byte1 ->                                            | discards the top 4 bytes (32-bits) of the stack                                    |
| pop8     | 0x06         | 1    | N/A               | byte8, ..., byte1 ->                                            | discards the top 8 bytes (64-bits) of the stack                                    |
| push     | 0x07         | 2    | byte              | -> byte                                                         | pushes byte (8-bits) onto the stack                                                |
| push2    | 0x08         | 3    | byte1, byte2      | -> byte2, byte1                                                 | pushes 2 bytes (16-bits) onto the stack                                            |
| push4    | 0x09         | 5    | byte1, ..., byte4 | -> byte4, ..., byte1                                            | pushes 4 bytes (32-bits) onto the stack                                            |
| push8    | 0x0A         | 9    | byte1, ..., byte8 | -> byte8, ..., byte1                                            | pushes 8 bytes (64-bits) onto the stack                                            |
| dup      | 0x0B         | 1    | N/A               | byte -> byte, byte                                              | duplicates the top byte of the stack                                               |
| dup2     | 0x0C         | 1    | N/A               | {byte2, byte1} -> {byte2, byte1}, {byte2, byte1}                | duplicates the top 2 bytes of the stack                                            |
| dup4     | 0x0D         | 1    | N/A               | {byte1, ..., byte4} -> {byte1, ..., byte4}, {byte1, ..., byte4} | duplicates the top 4 bytes of the stack                                            |
| dup8     | 0x0E         | 1    | N/A               | {byte1, ..., byte8} -> {byte1, ..., byte8}, {byte1, ..., byte8} | duplicates the top 8 bytes of the stack                                            |
| gsetr    | 0x0F         | 10   | index             | ref ->                                                          | stores a reference value (8 bytes) into global variable #index                     |
| gsetb    | 0x10         | 3    | index             | byte ->                                                         | stores a boolean value (1 byte) into global variable #index                        |
| gset8    | 0x11         | 3    | index             | byte ->                                                         | stores a byte value (1 byte) into global variable #index                           |
| gset16   | 0x12         | 4    | index             | byte2, byte1 ->                                                 | stores a short value (2 bytes) into global variable #index                         |
| gset32   | 0x13         | 6    | index             | byte4, ..., byte1 ->                                            | stores a int value (4 bytes) into global variable #index                           |
| gset64   | 0x14         | 10   | index             | byte8, ..., byte1 ->                                            | stores a long value (8 bytes) into global variable #index                          |
| gsetf    | 0x15         | 10   | index             | byte4, ..., byte1 ->                                            | stores a float value (4 bytes) into global variable #index                         |
| gsetd    | 0x16         | 10   | index             | byte8, ..., byte1 ->                                            | stores a double value (8 bytes) into global variable #index                        |
| gunset   | 0x17         | 2    | index             | N/A                                                             | unsets global variable #index                                                      |
| gisset   | 0x18         | 2    | index             | -> byte                                                         | checks if global variable #index is set, and pushes a boolean value onto the stack |
| lsetr    | 0x19         | 10   | index             | ref ->                                                          | stores a reference value (8 bytes) into local variable #index                      |
| lsetb    | 0x1A         | 3    | index             | byte ->                                                         | stores a boolean value (1 byte) into local variable #index                         |
| lset8    | 0x1B         | 3    | index             | byte ->                                                         | stores a byte value (1 byte) into local variable #index                            |
| lset16   | 0x1C         | 4    | index             | byte2, byte1 ->                                                 | stores a short value (2 bytes) into local variable #index                          |
| lset32   | 0x1D         | 6    | index             | byte4, ..., byte1 ->                                            | stores a int value (4 bytes) into local variable #index                            |
| lset64   | 0x1E         | 10   | index             | byte8, ..., byte1 ->                                            | stores a long value (8 bytes) into local variable #index                           |
| lsetf    | 0x1F         | 10   | index             | byte4, ..., byte1 ->                                            | stores a float value (4 bytes) into local variable #index                          |
| lsetd    | 0x20         | 10   | index             | byte8, ..., byte1 ->                                            | stores a double value (8 bytes) into local variable #index                         |
| lunset   | 0x21         | 2    | index             | N/A                                                             | unsets local variable #index                                                       |
| lisset   | 0x22         | 2    | index             | -> byte                                                         | checks if local variable #index is set, and pushes a boolean value onto the stack  |
| gloadr   | 0x23         | 2    | index             | -> ref                                                          | loads a reference value (8 bytes) from global variable #index                      |
| gloadb   | 0x24         | 2    | index             | -> byte                                                         | loads a boolean value (1 byte) from global variable #index                         |
| gload8   | 0x25         | 2    | index             | -> byte                                                         | loads a byte value (1 byte) from global variable #index                            |
| gload16  | 0x26         | 2    | index             | -> byte2, byte1                                                 | loads a short value (2 bytes) from global variable #index                          |
| gload32  | 0x27         | 2    | index             | -> byte4, ..., byte1                                            | loads a int value (4 bytes) from global variable #index                            |
| gload64  | 0x28         | 2    | index             | -> byte8, ..., byte1                                            | loads a long value (8 bytes) from global variable #index                           |
| gloadf   | 0x29         | 2    | index             | -> byte4, ..., byte1                                            | loads a float value (4 bytes) from global variable #index                          |
| gloadd   | 0x2A         | 2    | index             | -> byte8, ..., byte1                                            | loads a double value (8 bytes) from global variable #index                         |
| lloadr   | 0x2B         | 2    | index             | -> ref                                                          | loads a reference value (8 bytes) from local variable #index                       |
| lloadb   | 0x2C         | 2    | index             | -> byte                                                         | loads a boolean value (1 byte) from local variable #index                          |
| lload8   | 0x2D         | 2    | index             | -> byte                                                         | loads a byte value (1 byte) from local variable #index                             |
| lload16  | 0x2E         | 2    | index             | -> byte2, byte1                                                 | loads a short value (2 bytes) from local variable #index                           |
| lload32  | 0x2F         | 2    | index             | -> byte4, ..., byte1                                            | loads a int value (4 bytes) from local variable #index                             |
| lload64  | 0x30         | 2    | index             | -> byte8, ..., byte1                                            | loads a long value (8 bytes) from local variable #index                            |
| lloadf   | 0x31         | 2    | index             | -> byte4, ..., byte1                                            | loads a float value (4 bytes) from local variable #index                           |
| lloadd   | 0x32         | 2    | index             | -> byte8, ..., byte1                                            | loads a double value (8 bytes) from local variable #index                          |
