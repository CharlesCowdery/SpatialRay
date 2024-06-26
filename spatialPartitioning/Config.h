#pragma once

#define DRAW_REFLECTIVE_COEFFICIENTS 0
#define DRAW_DIFFUSE_COEFFICIENTS 0
#define DRAW_LIGHT_EXPOSURE 0
#define DRAW_COLOR 0
#define DEPTH_VIEW 0
#define DRAW_UV 0
#define DRAW_BOUNCE_DIRECTION 0
#define DRAW_NORMAL 0
#define DRAW_NORMAL_DELTAS 0
#define DRAW_NORMAL_FACING 0
#define DRAW_EMISSIVE 0

#define DRAW_LIGHTS   (1 && !DRAW_REFLECTIVE_COEFFICIENTS && !DRAW_DIFFUSE_COEFFICIENTS)
#define DRAW_DIFFUSE  (1 && !DRAW_REFLECTIVE_COEFFICIENTS)
#define DRAW_SPECULAR (1 && !DRAW_DIFFUSE_COEFFICIENTS)

#define USE_AVX_BVH 1
#define USE_AVX_TRI (1 && USE_AVX_BVH)
#define ENABLE_MIS  0
#define USE_PACKET_TRAVERSAL 1

#if USE_AVX_TRI
#define LEAF_SIZE 8
#define PENALIZE_UNFILLED_LEAFS 1
#else
#define LEAF_SIZE 2
#define PENALIZE_UNFILLED_LEAFS 0
#endif

#define IOR_STACK_SIZE 5
#define PDF_LOOKUP_RESOLUTION 1024
#define PDF_INTEGRAL_RESOLUTION (1024)
