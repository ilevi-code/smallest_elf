#include <elf.h>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include <iostream>
#include <iterator>
#include <iomanip>
#include <sys/stat.h>

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
    int fd = open(filename, O_WRONLY | O_CREAT, 0775);
    write(fd, &ehdr, sizeof(ehdr));
    write(fd, &phdr, sizeof(phdr));
    write(fd, segment.data(), segment.size());
    close(fd);
}
#include <arpa/inet.h>
#ifndef offsetof
#define offsetof(type, field) (&(((type*)NULL)->field))
#endif

int main() {
    constexpr const int LOAD_ADDR = 0x08048000;
    std::vector<uint8_t> text_section;

    read_into("main.text", text_section);

    Elf32_Phdr phdr;
    phdr.p_type = PT_LOAD;
    phdr.p_offset = 0;
    phdr.p_vaddr = LOAD_ADDR;
    phdr.p_paddr = htonl(0x0404eb10); // add $0x4,%al ; jmp +0x10 (end of phdr, start of instructions)
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
    hdr.e_entry = LOAD_ADDR + sizeof(hdr) + offsetof(Elf32_Phdr, p_paddr);
    hdr.e_phoff = sizeof(hdr);
    // the following are 2 uint32 fields, which can contain all values without affecting
    // program loading. posssibly j
    hdr.e_shoff = 0;
    hdr.e_flags = 0;
    hdr.e_ehsize = sizeof(hdr);
    hdr.e_phentsize = sizeof(Elf32_Phdr);
    hdr.e_phnum = 1;
    hdr.e_shentsize = sizeof(Elf32_Shdr);
    hdr.e_shnum = 0;
    hdr.e_shstrndx = SHN_UNDEF;

    write_out("final", hdr, phdr, text_section);
}
