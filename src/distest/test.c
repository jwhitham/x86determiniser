
#include <config.h>

#include <stdio.h>
#include <string.h>
#include <dis-asm.h>

char * stpcpy (char * dest, const char * src)
{
   while (*src) {
      *dest = *src;
      dest ++;
      src ++;
   }
   *dest = '\0';
   return dest;
}

int print_insn_i386 (bfd_vma pc, disassemble_info *info);

//enum bfd_architecture bfd_get_arch (bfd * x) { (void) x; return 0; }
//unsigned long bfd_get_mach (bfd * x) { (void) x; return 0; }

/* Function called to check if a SYMBOL is can be displayed to the user.
 This is used by some ports that want to hide special symbols when
 displaying debugging outout.  */
static bfd_boolean symbol_is_valid (asymbol *s, struct disassemble_info *dinfo)
{
    (void) s;
    (void) dinfo;
    return FALSE;
}

/* Function called to determine if there is a symbol at the given ADDR.
 If there is, the function returns 1, otherwise it returns 0.
 This is used by ports which support an overlay manager where
 the overlay number is held in the top part of an address.  In
 some circumstances we want to include the overlay number in the
 address, (normally because there is a symbol associated with
 that address), but sometimes we want to mask out the overlay bits.  */
static int symbol_at_address_func (bfd_vma addr, struct disassemble_info *dinfo)
{
    (void) addr;
    (void) dinfo;
    return 0;
}

/* Function called to print ADDR.  */
static void print_address_func (bfd_vma addr, struct disassemble_info *dinfo)
{
    dinfo->fprintf_func (dinfo->stream, "%p", (void *) addr);
}

/* Function which should be called if we get an error that we can't
 recover from.  STATUS is the errno value from read_memory_func and
 MEMADDR is the address that we were trying to read.  INFO is a
 pointer to this struct.  */
static void memory_error_func (int status, bfd_vma memaddr, struct disassemble_info *dinfo)
{
    (void) dinfo;
    printf ("memory_error_func status %d addr %p\n", status, (void *) memaddr);
}

/* Function used to get bytes to disassemble.  MEMADDR is the
 address of the stuff to be disassembled, MYADDR is the address to
 put the bytes in, and LENGTH is the number of bytes to read.
 INFO is a pointer to this struct.
 Returns an errno value or 0 for success.  */
static int read_memory_func (bfd_vma memaddr, bfd_byte *myaddr, unsigned int length, struct disassemble_info *dinfo)
{
    unsigned i;
    for (i = 0; i < length; i++) {
        switch (memaddr + i) {
            case 0x99: myaddr[i] = 0x8d; break;
            case 0x9a: myaddr[i] = 0x4c; break;
            case 0x9b: myaddr[i] = 0x24; break;
            case 0x9c: myaddr[i] = 0x0c; break;
            case 0x9d: myaddr[i] = 0x72; break;
            case 0x9e: myaddr[i] = 0x15; break;
            default:   myaddr[i] = 0x90; break;
        }
    }
    (void) memaddr;
    (void) dinfo;
    return 0;
}



int main (void)
{
    disassembler_ftype ft;
    struct disassemble_info di;
    int rc;

    memset (&di, 0, sizeof (di));
    di.read_memory_func = read_memory_func;
    di.memory_error_func = memory_error_func;
    di.print_address_func = print_address_func;
    di.symbol_at_address_func = symbol_at_address_func;
    di.symbol_is_valid = symbol_is_valid;
    di.mach = bfd_mach_i386_i386;     /* 32 bit */

    //typedef int (*fprintf_ftype) (void *, const char*, ...) ATTRIBUTE_FPTR_PRINTF_2;
    // fprintf_ftype fprintf_func;

    di.stream = stdout;
    di.fprintf_func = (fprintf_ftype) fprintf;
    ////////////////////
    
    ft = print_insn_i386;
    printf ("%p\n", ft);
    rc = ft (0x98, &di);
    printf ("\n%d %d\n", rc, di.insn_type);
    rc = ft (0x99, &di);
    printf ("\n%d %d\n", rc, di.insn_type);
    rc = ft (0x9d, &di);
    printf ("\n%d %d\n", rc, di.insn_type);


    return 0;
}


