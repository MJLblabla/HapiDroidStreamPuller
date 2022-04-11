#ifndef __AMF_CORE_H__
#define __AMF_CORE_H__

#define freep(p) \
    if (p) { \
        delete p; \
        p = nullptr; \
        } \
    (void)0

#define freea() \
    if (p) { \
        delete [] p; \
        p = nullptr; \
            } \
    (void)0

#endif
