#include "internal.h"
#include "ruby_assert.h"

#include <errno.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef MAXPATHLEN
# define MAXPATHLEN 1024
#endif

#define RUBY_HUGEPAGE_SIZE 2L * 1024 * 1024
const size_t ruby_hugepage_size = RUBY_HUGEPAGE_SIZE;

inline unsigned long ruby_hugepage_align_up(unsigned long addr) {
  return (((addr) + (ruby_hugepage_size) - 1) & ~((ruby_hugepage_size) - 1));
}

inline unsigned long ruby_hugepage_align_down(unsigned long addr) {
  return ((addr) & ~((ruby_hugepage_size) - 1));
}

struct rb_mem_segment {
  unsigned long start;
  unsigned long end;
  unsigned long offset;
  unsigned long inode;
  char perm[5];
  char dev[6];
  char name[MAXPATHLEN];
};

struct rb_hugepage_segment {
  char *start;
  char *end;
  size_t size;
  int pages;
};

struct rb_mem_segment
rb_hugepage_find_mem_segment(char *name, unsigned long size, char *perm)
{
  FILE *f;
  int ret;
  char line[257];
  struct rb_mem_segment s;
  memset(&s, 0, sizeof(struct rb_mem_segment));
  f = fopen("/proc/self/maps", "r");
	if (f) {
    while (!feof(f)) {
      ret = fscanf(f, "%lx-%lx %4s %lx %5s %ld %s\n", &s.start, &s.end, s.perm, &s.offset, s.dev, &s.inode, s.name);
      printf("%lu-%lu %4s %lu %5s %ld %s (%lu)\n", s.start, s.end, s.perm, s.offset, s.dev, s.inode, s.name, s.end - s.start);
      //sleep(1);
      if (ret == 7) {
        if (strcmp(perm, s.perm) == 0 && strcmp(name, s.name) == 0) {
          if (size != 0) {
            if ((s.end - s.start) == size) {
              goto found;
            }
          } else {
            goto found;
          }
        }
      } else {
        void *l = fgets(line, 257, f);
      }
    }
  }

  found:
    fclose(f);
    return s;
}

extern char __executable_start;
extern char _start;
extern char __etext;

struct rb_hugepage_segment
rb_hugepage_find_text_segment()
{
  char binary_filename[PATH_MAX];
  struct rb_mem_segment s;
  struct rb_hugepage_segment hs;
  memset(&s, 0, sizeof(struct rb_mem_segment));
  memset(&hs, 0, sizeof(struct rb_hugepage_segment));

  ssize_t len = readlink("/proc/self/exe", binary_filename, PATH_MAX);
  binary_filename[len] = 0;
  s = rb_hugepage_find_mem_segment(binary_filename, 0, "r-xp");

  hs.start = (char *)ruby_hugepage_align_up(s.start);
  hs.end = (char *)ruby_hugepage_align_down(s.end);
  printf("!!! %lu %lu (%lu) %lu %lu (%lu) - %lu %lu\n", s.end, s.start, (s.end - s.start), hs.end, hs.start, (hs.end - hs.start), (s.end - (long unsigned)hs.end), (s.start - (long unsigned)hs.start));

  //hs.start = (char *)ruby_hugepage_align_up(s.start);
  //hs.end = (char *)ruby_hugepage_align_down(s.end);
  if (hs.end > hs.start) {
    hs.size = hs.end - hs.start;
    hs.pages = hs.size / ruby_hugepage_size;
  }
  return hs;
}

#ifndef _WIN32
# include <sys/mman.h>
# ifndef MAP_FAILED
#  define MAP_FAILED ((void*)-1)
# endif
#endif

int
rb_hugepage_supported_p()
{
  struct rb_mem_segment s;
  memset(&s, 0, sizeof(struct rb_mem_segment));
  char *hp = mmap(NULL, ruby_hugepage_size,
             PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
            -1, 0);
  if (hp == MAP_FAILED) return -1;
  s = rb_hugepage_find_mem_segment("/anon_hugepage", ruby_hugepage_size, "rw-p");
  munmap(hp, ruby_hugepage_size);
  if ((char *)s.start == hp && (s.end - s.start) == ruby_hugepage_size){
    return 1;
  }

  return -1;
}

int
__attribute__((__section__(".hpmapper.text")))
//__attribute__((__aligned__(RUBY_HUGEPAGE_SIZE)))
__attribute__((__noinline__))
__attribute__((__optimize__("2")))
rb_move_region_to_hugepages(struct rb_hugepage_segment region)
{
  void *hps = MAP_FAILED;
  void *mem =  mmap(NULL, region.size,
             PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS,
            -1, 0);
  if (mem == NULL) return -1;

  memcpy(mem, region.start, region.size);

#ifdef MAP_HUGETLB
	hps = mmap(region.start, region.size,
	PROT_READ | PROT_WRITE,
	MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED | MAP_HUGETLB,
	-1, 0);
#endif
  //sleep(1000);
	if (hps == MAP_FAILED) {
    hps = mmap(region.start, region.size,
	  PROT_READ | PROT_WRITE,
	  MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
	  -1, 0);
    if (hps == MAP_FAILED) {
	  	rb_bug("hugepages");
    }
    memcpy(region.start, mem, region.size);
    mprotect(region.start, region.size, PROT_READ | PROT_EXEC);
    munmap(mem, region.size);
    return -1;
	}

  if (hps == region.start) {
    memcpy(region.start, mem, region.size);
    mprotect(region.start, region.size, PROT_READ | PROT_EXEC);
  }

  munmap(mem, region.size);

  return (hps == region.start) ? 1 : -1;
}

void
rb_hugepage_remap_static_code(){
  struct rb_hugepage_segment text;

  if (rb_hugepage_supported_p() == -1) {
    printf("hugepages not supported!\n");
    return;
  }
  text = rb_hugepage_find_text_segment();
  if (text.pages > 0) {
    if (rb_move_region_to_hugepages(text) == -1) {
      printf("hugepage mapping failed\n");
    }
  }
}