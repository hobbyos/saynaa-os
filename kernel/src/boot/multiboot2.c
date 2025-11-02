#include "kernel/boot/multiboot2.h"

#include "libc/math.h"

/* Returns the first multiboot2 tag of the requested type.
 */
mb2_tag_t* mb2_find_tag(mb2_t* boot, uint32_t tag_type) {
    mb2_tag_t* tag = boot->tags;
    mb2_tag_t* prev_tag = tag;

    do {
        if (tag->type == tag_type) {
            return tag;
        }

        prev_tag = tag;
        tag = (mb2_tag_t*) ((uintptr_t) tag + align_to(tag->size, 8));
    } while (prev_tag->type != MB2_TAG_END);

    return NULL;
}