#pragma once

#include "../../scanning/include/scanning/symbolfinder.hpp"
#include <dlfcn.h>
#include <link.h>
#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __x86_64__
  #define Elf_Ehdr Elf64_Ehdr
  #define Elf_Shdr Elf64_Shdr
  #define Elf_Sym  Elf64_Sym
#else
  #define Elf_Ehdr Elf32_Ehdr
  #define Elf_Shdr Elf32_Shdr
  #define Elf_Sym  Elf32_Sym
#endif

#ifndef ELF_ST_TYPE
  #define ELF_ST_TYPE(info) ((info) & 0xf)
#endif

class SymbolFinderEx : public SymbolFinder {
public:
  void* GetBinaryFromPartialPath(const char* partialPath) {
    printf("Searching for binary containing: %s\n", partialPath);
    
    void* handle = dlopen(nullptr, RTLD_NOW);
    if (!handle) {
      printf("Failed to get handle to program: %s\n", dlerror());
      return nullptr;
    }

    link_map* map;
    if (dlinfo(handle, RTLD_DI_LINKMAP, &map) == -1) {
      printf("Failed to get link map: %s\n", dlerror());
      dlclose(handle);
      return nullptr;
    }

    while (map != nullptr) {
      if (strstr(map->l_name, partialPath) != nullptr) {
        printf("Found matching binary at: %s\n", map->l_name);
        void* binary = dlopen(map->l_name, RTLD_LAZY | RTLD_NOLOAD);
        dlclose(handle);
        return binary;
      }
      map = map->l_next;
    }

    dlclose(handle);
    printf("Binary not found\n");
    return nullptr;
  }

  void* FindSymbolFromBinaryPartial(const char* partialPath, const char* symbol) {
    void* binary = GetBinaryFromPartialPath(partialPath);
    if (binary != nullptr) {
      void* symbol_pointer = FindSymbol(binary, symbol);
      dlclose(binary);
      return symbol_pointer;
    }
    return nullptr;
  }

  void* FindSymbolEx(const char* partialPath, const char* symbol) {
    void* binary = GetBinaryFromPartialPath(partialPath);
    if (!binary) {
      printf("Binary not found\n");
      return nullptr;
    }

    void* found_address = nullptr;
    const struct link_map* dlmap = reinterpret_cast<const struct link_map*>(binary);
    struct stat64 dlstat;
    int dlfile = open(dlmap->l_name, O_RDONLY);
    
    if (dlfile != -1 && fstat64(dlfile, &dlstat) != -1) {
      Elf_Ehdr* file_hdr = reinterpret_cast<Elf_Ehdr*>(mmap(0, dlstat.st_size, PROT_READ, MAP_PRIVATE, dlfile, 0));
      if (file_hdr != MAP_FAILED) {
        if (file_hdr->e_shoff != 0 && file_hdr->e_shstrndx != SHN_UNDEF) {
          uintptr_t map_base = reinterpret_cast<uintptr_t>(file_hdr);
          Elf_Shdr* sections = reinterpret_cast<Elf_Shdr*>(map_base + file_hdr->e_shoff);
          Elf_Shdr* shstrtab_hdr = &sections[file_hdr->e_shstrndx];
          const char* shstrtab = reinterpret_cast<const char*>(map_base + shstrtab_hdr->sh_offset);

          Elf_Shdr* symtab_hdr = nullptr, *strtab_hdr = nullptr;
          for (uint16_t i = 0; i < file_hdr->e_shnum; i++) {
            const char* section_name = shstrtab + sections[i].sh_name;
            if (strcmp(section_name, ".symtab") == 0)
              symtab_hdr = &sections[i];
            else if (strcmp(section_name, ".strtab") == 0)
              strtab_hdr = &sections[i];
          }

          if (symtab_hdr && strtab_hdr) {
            Elf_Sym* symtab = reinterpret_cast<Elf_Sym*>(map_base + symtab_hdr->sh_offset);
            const char* strtab = reinterpret_cast<const char*>(map_base + strtab_hdr->sh_offset);
            uint32_t symbol_count = symtab_hdr->sh_size / symtab_hdr->sh_entsize;

            for (uint32_t i = 0; i < symbol_count; i++) {
              Elf_Sym& sym = symtab[i];
              uint8_t sym_type = ELF_ST_TYPE(sym.st_info);
              const char* sym_name = strtab + sym.st_name;

              if (sym.st_shndx != SHN_UNDEF && 
                (sym_type == STT_FUNC || sym_type == STT_OBJECT)) {
                if (strcmp(sym_name, symbol) == 0) {
                  found_address = reinterpret_cast<void*>(dlmap->l_addr + sym.st_value);
                  //printf("Found symbol %s at %p\n", sym_name, found_address);
                  break;
                }
              }
            }
          }
        }
        munmap(file_hdr, dlstat.st_size);
      }
      close(dlfile);
    }
    
    dlclose(binary);
    return found_address;
  }

  void* ListAllBinaries() {
    void* handle = dlopen(nullptr, RTLD_NOW);
    if (!handle) {
      printf("Failed to get handle to program: %s\n", dlerror());
      return nullptr;
    }

    link_map* map;
    if (dlinfo(handle, RTLD_DI_LINKMAP, &map) == -1) {
      printf("Failed to get link map: %s\n", dlerror());
      dlclose(handle);
      return nullptr;
    }

    printf("Loaded binaries:\n");
    while (map != nullptr) {
      printf("  %p: %s\n", (void*)map->l_addr, map->l_name);
      map = map->l_next;
    }

    dlclose(handle);
    return nullptr;
  }

  void ListSymbolsContaining(const char* partialPath, const char* symbolPart) {
    printf("Searching for symbols containing '%s' in binary: %s\n", symbolPart, partialPath);
    
    void* binary = GetBinaryFromPartialPath(partialPath);
    if (!binary) {
      printf("Binary not found\n");
      return;
    }

    const struct link_map* dlmap = reinterpret_cast<const struct link_map*>(binary);
    struct stat64 dlstat;
    int dlfile = open(dlmap->l_name, O_RDONLY);
    if (dlfile != -1 && fstat64(dlfile, &dlstat) != -1) {
      Elf_Ehdr* file_hdr = reinterpret_cast<Elf_Ehdr*>(mmap(0, dlstat.st_size, PROT_READ, MAP_PRIVATE, dlfile, 0));
      if (file_hdr != MAP_FAILED) {
        if (file_hdr->e_shoff != 0 && file_hdr->e_shstrndx != SHN_UNDEF) {
          uintptr_t map_base = reinterpret_cast<uintptr_t>(file_hdr);
          Elf_Shdr* sections = reinterpret_cast<Elf_Shdr*>(map_base + file_hdr->e_shoff);
          Elf_Shdr* shstrtab_hdr = &sections[file_hdr->e_shstrndx];
          const char* shstrtab = reinterpret_cast<const char*>(map_base + shstrtab_hdr->sh_offset);

          Elf_Shdr* symtab_hdr = nullptr, *strtab_hdr = nullptr;
          for (uint16_t i = 0; i < file_hdr->e_shnum; i++) {
            const char* section_name = shstrtab + sections[i].sh_name;
            if (strcmp(section_name, ".symtab") == 0)
              symtab_hdr = &sections[i];
            else if (strcmp(section_name, ".strtab") == 0)
              strtab_hdr = &sections[i];
          }

          if (symtab_hdr && strtab_hdr) {
            Elf_Sym* symtab = reinterpret_cast<Elf_Sym*>(map_base + symtab_hdr->sh_offset);
            const char* strtab = reinterpret_cast<const char*>(map_base + strtab_hdr->sh_offset);
            uint32_t symbol_count = symtab_hdr->sh_size / symtab_hdr->sh_entsize;

            printf("Scanning %u symbols for matches...\n", symbol_count);
            for (uint32_t i = 0; i < symbol_count; i++) {
              Elf_Sym& sym = symtab[i];
              uint8_t sym_type = ELF_ST_TYPE(sym.st_info);
              const char* sym_name = strtab + sym.st_name;

              if (sym.st_shndx != SHN_UNDEF && (sym_type == STT_FUNC || sym_type == STT_OBJECT)) {
                if (strstr(sym_name, symbolPart) != nullptr) {
                  void* symaddr = reinterpret_cast<void*>(dlmap->l_addr + sym.st_value);
                  printf("Found matching symbol: %s at %p\n", sym_name, symaddr);
                }
              }
            }
          }
        }
        munmap(file_hdr, dlstat.st_size);
      }
      close(dlfile);
    }
    
    dlclose(binary);
  }
};