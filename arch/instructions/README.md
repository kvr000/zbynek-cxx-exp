# Instructions comparison for various architectures

## Logical

src/main/cxx/Logical.cxx

Testing simple condition for checking boundaries:

```
        return ((pos+7)&((BLOCK_SIZE-1)&~7));
```

- x86_64 does not have major problems, as it can represent any immediate value, though the length increases.  Two instructions are sufficient (excluding branch or condition set).
- aarch64 leverages smart bitmask encoding and therefore is able to fit the mask into single instruction with `tst immediate` instruction.  Two instructions are sufficient (excluding branch or condition set).
- Most other RISC architectures need to load the operand first into a register and perform operation on two registers.  This usually requires four instructions.
