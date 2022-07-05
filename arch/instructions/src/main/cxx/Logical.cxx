#define BLOCK_SIZE (1024*1024*1024)


// aarch64:
// 1: 1f 71 7f f2       tst     x8, #0x3ffffffe         N=1 immr=63 imms=28 rn=8 rd=31
// 3: 1f 6d 7e f2       tst     x8, #0x3ffffffc         N=1 immr=62 imms=27 rn=8 rd=31
// 7: 1f 69 7d f2       tst     x8, #0x3ffffff8         N=1 immr=61 imms=26 rn=8 rd=31
// f: 1f 65 7c f2       tst     x8, #0x3ffffff0         N=1 immr=60 imms=25 rn=8 rd=31
bool logical_cmpAddFirst(long pos)
{
        return ((pos+7)&((BLOCK_SIZE-1)&~7));
}

bool logical_cmpAddLast(long pos)
{
        return ((pos)&(BLOCK_SIZE-1)) <= BLOCK_SIZE-8;
}

volatile int logical_x;

void logicalFunc()
{
        if (logical_cmpAddFirst(logical_x)) {
                logical_x = 11;
        }
        if (logical_cmpAddLast(logical_x)) {
                logical_x = 12;
        }
}
