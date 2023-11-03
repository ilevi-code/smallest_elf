#include <elf.h>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include <iostream>
#include <iterator>
#include <iomanip>
#include <sys/stat.h>
#include <arpa/inet.h>

#ifndef offsetof
#define offsetof(type, field) (&(((type*)NULL)->field))
#endif
#define htonll(x) ((((uint64_t)htonl(x & 0xffffffff)) << 32) + htonl((x) >> 32))

void read_into(const char* filename, std::vector<uint8_t>& out)
{
    int fd = open(filename, O_RDONLY);
    struct stat stats;
    fstat(fd, &stats);
    out.resize(stats.st_size);
    auto nread = read(fd, out.data(), out.capacity());
    close(fd);
}

void write_out(const char* filename, const Elf32_Ehdr& ehdr, const Elf32_Phdr& phdr, const std::vector<uint8_t>& segment)
{
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0775);
    write(fd, &ehdr, sizeof(ehdr));
    write(fd, &phdr, sizeof(phdr));
    write(fd, segment.data(), segment.size());
    close(fd);
}

int main() {
    std::vector<uint8_t> text_section;

    read_into("main.text", text_section);

    Elf32_Phdr phdr;
    phdr.p_type = PT_LOAD;
    phdr.p_offset = 0;
    // the lower 2 bytes of the loading address are also instructions.
    // 43   | inc %ebx
    // cd80 | int 0x80
    // 93   | xchg %ebx, %eax
    constexpr const uint32_t LOAD_ADDR = 0xcd438000;
    phdr.p_vaddr = LOAD_ADDR;
    phdr.p_paddr = htonl(0x8093eb10);
    phdr.p_filesz = sizeof(phdr) + sizeof(Elf32_Ehdr) + text_section.size();
    phdr.p_memsz = sizeof(phdr) + sizeof(Elf32_Ehdr) + text_section.size();
    phdr.p_flags = PF_X | PF_R;
    phdr.p_align = 0x1000;

    Elf32_Ehdr hdr;
    hdr.e_ident[EI_MAG0] = ELFMAG0;
    hdr.e_ident[EI_MAG1] = ELFMAG1;
    hdr.e_ident[EI_MAG2] = ELFMAG2;
    hdr.e_ident[EI_MAG3] = ELFMAG3;
    hdr.e_ident[EI_CLASS] = ELFCLASS32;
    hdr.e_ident[EI_DATA] = ELFDATA2LSB;
    hdr.e_ident[EI_VERSION] = EV_CURRENT;
    hdr.e_ident[EI_OSABI] = ELFOSABI_SYSV;
    hdr.e_ident[EI_ABIVERSION] = 0;
    memset(&hdr.e_ident[EI_PAD], 0, EI_NIDENT - EI_PAD);
    hdr.e_type = ET_EXEC;
    hdr.e_machine = EM_386;
    hdr.e_version = EV_CURRENT;
    hdr.e_entry = LOAD_ADDR + offsetof(Elf32_Ehdr, e_shoff);
    hdr.e_phoff = sizeof(hdr);
    // both e_shoff and e_flags are uint32 fields, which can contain all that
    // are irrelevant to the loader. We fill them with:
    // 81c1588043cd | add $0x4304805f, %ecx
    // eb06         | jmp +0x18 (start of p_paddr)
    *((uint64_t*)&hdr.e_shoff) = htonll(0x81c1588043cdeb06ULL);
    hdr.e_ehsize = sizeof(hdr);
    hdr.e_phentsize = sizeof(Elf32_Phdr);
    hdr.e_phnum = 1;
    // e_shentsize, e_shnum and e_shstrndx are irrelevant. Thats 6 more bytes
    // 0404 | mov $0x4,%al
    // b20d | mov $0xd,%dl
    // jmp +0xa (middle of p_vaddr)
    *((uint32_t*)&hdr.e_shentsize) = htonl(0x0404b20d);
    hdr.e_shstrndx = htons(0xeb0a);

    write_out("final", hdr, phdr, text_section);
}
