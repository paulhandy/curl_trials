/*
 * Copyright (c) 2016, Shinya Yagyu
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <malloc.h>
#endif /* defined(_MSC_VER) || defined(__MINGW32__) */

#define HBITS 0xFFFFFFFFFFFFFFFFL
#define LBITS 0x0000000000000000L
#define HASH_LENGTH 243              //trits
#define STATE_LENGTH 3 * HASH_LENGTH //trits
#define HALF_LENGTH 364              //trits
#define TX_LENGTH 2673               //trytes

#define LOW00 0xDB6DB6DB6DB6DB6DL  //0b1101101101101101101101101101101101101101101101101101101101101101L;
#define HIGH00 0xB6DB6DB6DB6DB6DBL //0b1011011011011011011011011011011011011011011011011011011011011011L;
#define LOW10 0xF1F8FC7E3F1F8FC7L  //0b1111000111111000111111000111111000111111000111111000111111000111L;
#define HIGH10 0x8FC7E3F1F8FC7E3FL //0b1000111111000111111000111111000111111000111111000111111000111111L;
#define LOW20 0x7FFFE00FFFFC01FFL  //0b0111111111111111111000000000111111111111111111000000000111111111L;
#define HIGH20 0xFFC01FFFF803FFFFL //0b1111111111000000000111111111111111111000000000111111111111111111L;
#define LOW30 0xFFC0000007FFFFFFL  //0b1111111111000000000000000000000000000111111111111111111111111111L;
#define HIGH30 0x003FFFFFFFFFFFFFL //0b0000000000111111111111111111111111111111111111111111111111111111L;
#define LOW40 0xFFFFFFFFFFFFFFFFL  //0b1111111111111111111111111111111111111111111111111111111111111111L;
#define HIGH40 0xFFFFFFFFFFFFFFFFL //0b1111111111111111111111111111111111111111111111111111111111111111L;

#define LOW01 0x6DB6DB6DB6DB6DB6L
#define HIGH01 0xDB6DB6DB6DB6DB6CL
#define LOW11 0xF8FC7E3F1F8FC7E2L
#define HIGH11 0xC7E3F1F8FC7E3F1EL
#define LOW21 0xC01FFFF803FFFF00L
#define HIGH21 0x3FFFF007FFFE00FEL
#define LOW31 0xFFFFF0000001FFFEL
#define HIGH31 0xFFFFF00000000000L
#define LOW41 0x00000FFFFFFFFFFEL
#define HIGH41 0xFFFFFFFFFFFFFFFFL //0b1111111111111111111111111111111111111111111111111111111111111111L;

#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 201102L
#ifdef _MSC_VER
#define _Alignas(n) __declspec(align(n))
#else
#define _Alignas(n) __attribute__((aligned(n)))
#endif // _MSC_VER
#endif // defined(__cplusplus) && __cplusplus < 201103L

//	 _Alignas(ALIGN) unsigned char array[10] = {0};

#if defined(__cplusplus) && __cplusplus < 201103L
#ifdef _MSC_VER
#define alignof(n) __alignof(n)
#else
#define alignof(n) __alignof__(n)
#endif // _MSC_VER
#endif // defined(__cplusplus) || cplusplus < 201103L

#if defined(ENABLE_AVX512)
#define ALIGN alignof(__m512)
#elif defined(ENABLE_AVX)
#define ALIGN alignof(__m256)
#elif defined(ENABLE_SSE)
#define ALIGN alignof(__m128)
#elif defined(ENABLE_NEON)
#define ALIGN alignof(float32x4_t)
#else
#define ALIGN 8
#endif // defined(ENABLE_AVX512)

int indices[] = {
    0, 364, 728, 363, 727, 362, 726, 361, 725, 360, 724, 359, 723, 358, 722, 357, 721, 356, 720, 355, 719, 354, 718, 353, 717, 352, 716, 351, 715, 350, 714, 349, 713, 348, 712, 347, 711, 346, 710, 345, 709, 344, 708, 343, 707, 342, 706, 341, 705, 340, 704, 339, 703, 338, 702, 337, 701, 336, 700, 335, 699, 334, 698, 333, 697, 332, 696, 331, 695, 330, 694, 329, 693, 328, 692, 327, 691, 326, 690, 325, 689, 324, 688, 323, 687, 322, 686, 321, 685, 320, 684, 319, 683, 318, 682, 317, 681, 316, 680, 315, 679, 314, 678, 313, 677, 312, 676, 311, 675, 310, 674, 309, 673, 308, 672, 307, 671, 306, 670, 305, 669, 304, 668, 303, 667, 302, 666, 301, 665, 300, 664, 299, 663, 298, 662, 297, 661, 296, 660, 295, 659, 294, 658, 293, 657, 292, 656, 291, 655, 290, 654, 289, 653, 288, 652, 287, 651, 286, 650, 285, 649, 284, 648, 283, 647, 282, 646, 281, 645, 280, 644, 279, 643, 278, 642, 277, 641, 276, 640, 275, 639, 274, 638, 273, 637, 272, 636, 271, 635, 270, 634, 269, 633, 268, 632, 267, 631, 266, 630, 265, 629, 264, 628, 263, 627, 262, 626, 261, 625, 260, 624, 259, 623, 258, 622, 257, 621, 256, 620, 255, 619, 254, 618, 253, 617, 252, 616, 251, 615, 250, 614, 249, 613, 248, 612, 247, 611, 246, 610, 245, 609, 244, 608, 243, 607, 242, 606, 241, 605, 240, 604, 239, 603, 238, 602, 237, 601, 236, 600, 235, 599, 234, 598, 233, 597, 232, 596, 231, 595, 230, 594, 229, 593, 228, 592, 227, 591, 226, 590, 225, 589, 224, 588, 223, 587, 222, 586, 221, 585, 220, 584, 219, 583, 218, 582, 217, 581, 216, 580, 215, 579, 214, 578, 213, 577, 212, 576, 211, 575, 210, 574, 209, 573, 208, 572, 207, 571, 206, 570, 205, 569, 204, 568, 203, 567, 202, 566, 201, 565, 200, 564, 199, 563, 198, 562, 197, 561, 196, 560, 195, 559, 194, 558, 193, 557, 192, 556, 191, 555, 190, 554, 189, 553, 188, 552, 187, 551, 186, 550, 185, 549, 184, 548, 183, 547, 182, 546, 181, 545, 180, 544, 179, 543, 178, 542, 177, 541, 176, 540, 175, 539, 174, 538, 173, 537, 172, 536, 171, 535, 170, 534, 169, 533, 168, 532, 167, 531, 166, 530, 165, 529, 164, 528, 163, 527, 162, 526, 161, 525, 160, 524, 159, 523, 158, 522, 157, 521, 156, 520, 155, 519, 154, 518, 153, 517, 152, 516, 151, 515, 150, 514, 149, 513, 148, 512, 147, 511, 146, 510, 145, 509, 144, 508, 143, 507, 142, 506, 141, 505, 140, 504, 139, 503, 138, 502, 137, 501, 136, 500, 135, 499, 134, 498, 133, 497, 132, 496, 131, 495, 130, 494, 129, 493, 128, 492, 127, 491, 126, 490, 125, 489, 124, 488, 123, 487, 122, 486, 121, 485, 120, 484, 119, 483, 118, 482, 117, 481, 116, 480, 115, 479, 114, 478, 113, 477, 112, 476, 111, 475, 110, 474, 109, 473, 108, 472, 107, 471, 106, 470, 105, 469, 104, 468, 103, 467, 102, 466, 101, 465, 100, 464, 99, 463, 98, 462, 97, 461, 96, 460, 95, 459, 94, 458, 93, 457, 92, 456, 91, 455, 90, 454, 89, 453, 88, 452, 87, 451, 86, 450, 85, 449, 84, 448, 83, 447, 82, 446, 81, 445, 80, 444, 79, 443, 78, 442, 77, 441, 76, 440, 75, 439, 74, 438, 73, 437, 72, 436, 71, 435, 70, 434, 69, 433, 68, 432, 67, 431, 66, 430, 65, 429, 64, 428, 63, 427, 62, 426, 61, 425, 60, 424, 59, 423, 58, 422, 57, 421, 56, 420, 55, 419, 54, 418, 53, 417, 52, 416, 51, 415, 50, 414, 49, 413, 48, 412, 47, 411, 46, 410, 45, 409, 44, 408, 43, 407, 42, 406, 41, 405, 40, 404, 39, 403, 38, 402, 37, 401, 36, 400, 35, 399, 34, 398, 33, 397, 32, 396, 31, 395, 30, 394, 29, 393, 28, 392, 27, 391, 26, 390, 25, 389, 24, 388, 23, 387, 22, 386, 21, 385, 20, 384, 19, 383, 18, 382, 17, 381, 16, 380, 15, 379, 14, 378, 13, 377, 12, 376, 11, 375, 10, 374, 9, 373, 8, 372, 7, 371, 6, 370, 5, 369, 4, 368, 3, 367, 2, 366, 1, 365, 0};

char truthTable[] = {1, 0, -1, 0, 1, -1, 0, 0, -1, 1, 0};

char tryte2trit_table[29][3] = {
    {0, 0, 0}, {1, 0, 0}, {-1, 1, 0}, {0, 1, 0}, {1, 1, 0}, {-1, -1, 1}, {0, -1, 1}, {1, -1, 1}, {-1, 0, 1}, {0, 0, 1}, {1, 0, 1}, {-1, 1, 1}, {0, 1, 1}, {1, 1, 1}, {-1, -1, -1}, {0, -1, -1}, {1, -1, -1}, {-1, 0, -1}, {0, 0, -1}, {1, 0, -1}, {-1, 1, -1}, {0, 1, -1}, {1, 1, -1}, {-1, -1, 0}, {0, -1, 0}, {1, -1, 0}, {-1, 0, 0}};

void tx2trit(char tryte[], char trit[])
{
  int i = 0;
  if (strlen(tryte) != TX_LENGTH)
  {
    fprintf(stderr, "illegal tx length.\n");
    exit(1);
  }
  for (i = 0; i < TX_LENGTH; i++)
  {
    if (!(tryte[i] == '9' || (tryte[i] >= 'A' && tryte[i] <= 'Z')))
    {
      fprintf(stderr, "illegal tryte.\n");
      exit(1);
    }
    int ch = 0;
    if (tryte[i] != '9')
    {
      ch = tryte[i] - 'A' + 1;
    }
    trit[i * 3] = tryte2trit_table[ch][0];
    trit[i * 3 + 1] = tryte2trit_table[ch][1];
    trit[i * 3 + 2] = tryte2trit_table[ch][2];
  }
}

void hash2tryte(char trit[], char tryte[])
{
  int i = 0;
  for (i = 0; i < HASH_LENGTH / 3; i++)
  {
    int n = trit[i * 3] + trit[i * 3 + 1] * 3 + trit[i * 3 + 2] * 9;
    if (n < 0)
    {
      n += 27;
    }
    if (n > 27 || n < 0)
    {
      fprintf(stderr, "illegal trit.\n");
      exit(1);
    }
    if (n == 0)
    {
      tryte[i] = '9';
    }
    else
    {
      tryte[i] = n + 'A' - 1;
    }
  }
  tryte[HASH_LENGTH / 3] = 0;
}

void transform(char state[])
{
  int r = 0, i = 0;
  char copy[STATE_LENGTH] = {0};
  for (r = 0; r < 27; r++)
  {
    memcpy(copy, state, STATE_LENGTH);
    for (i = 0; i < STATE_LENGTH; i++)
    {
      int aa = indices[i];
      int bb = indices[i + 1];
      state[i] = truthTable[copy[aa] + (copy[bb] << 2) + 5];
    }
  }
}

void absorb(char state[], char in[], int size)
{
  int len = 0, offset = 0;
  for (offset = 0; offset < size; offset += len)
  {
    len = 243;
    if (size < 243)
    {
      len = size;
    }
    memcpy(state, in + offset, len);
    transform(state);
  }
}

void transform64(__m128i *lmid, __m128i *hmid)
{
  static int j, r, t1, t2; // static for speed
  __m128i alpha, beta, gamma, delta;
  __m128i ltmp[STATE_LENGTH], htmp[STATE_LENGTH];
  __m128i *lto = ltmp, *hto = htmp;
  __m128i *lfrom = lmid, *hfrom = hmid;
  for (r = 0; r < 27; r++)
  {
    for (j = 0; j < STATE_LENGTH; j++)
    {
      t1 = indices[j];
      t2 = indices[j + 1];

      alpha = lfrom[t1];
      beta = hfrom[t1];
      gamma = hfrom[t2];                               //3
      delta = (alpha | (~gamma)) & (lfrom[t2] ^ beta); //1

      lto[j] = ~delta; //6
      hto[j] = (alpha ^ gamma) | delta;
    }
    __m128i *lswap = lfrom, *hswap = hfrom;
    lfrom = lto;
    hfrom = hto;
    lto = lswap;
    hto = hswap;
  }
  for (j = 0; j < STATE_LENGTH; j++)
  {
    lmid[j] = ltmp[j];
    hmid[j] = htmp[j];
  }
}

int incr(__m128i *mid_low, __m128i *mid_high)
{
  int i;
  for (i = 5; i < HASH_LENGTH; i++)
  {
    if (mid_low[i][0] == LBITS)
    {
      mid_low[i] = _mm_set_epi64x(HBITS, HBITS);
      mid_high[i] = _mm_set_epi64x(LBITS, LBITS);
    }
    else
    {
      if (mid_high[i][0] == LBITS)
      {
        mid_high[i] = _mm_set_epi64x(HBITS, HBITS);
      }
      else
      {
        mid_low[i] = _mm_set_epi64x(LBITS, LBITS);
      }
      return 0;
    }
  }
  return 1;
}

void seri(__m128i *low, __m128i *high, int n, char *r)
{
  int i = 0, index = 0;
  if (n >= 64)
  {
    n -= 64;
    index = 1;
  }
  for (i = 0; i < HASH_LENGTH; i++)
  {
    __m128i ll128 = (low[i] >> n) & 1;
    __m128i hh128 = (high[i] >> n) & 1;
    _Alignas(ALIGN) long ll = ll128[index], hh = hh128[index];
    if (hh == 0 && ll == 1)
    {
      r[i] = -1;
    }
    if (hh == 1 && ll == 1)
    {
      r[i] = 0;
    }
    if (hh == 1 && ll == 0)
    {
      r[i] = 1;
    }
  }
}

int check(__m128i *l, __m128i *h, long m)
{
  int i, j; //omit init for speed

  __m128i nonce_probe = _mm_set_epi64x(HBITS, HBITS);
  for (i = HASH_LENGTH - m; i < HASH_LENGTH; i++)
  {
    nonce_probe &= ~(l[i] ^ h[i]);
    if (nonce_probe[0] == LBITS && nonce_probe[1] == LBITS)
    {
      return -1;
    }
  }
  for (j = 0; j < 2; j++)
  {
    for (i = 0; i < 64; i++)
    {
      if ((nonce_probe[j] >> i) & 1)
      {
        return i + j * 64;
      }
    }
  }
  return -2;
}

int loop_cpu(__m128i *lmid, __m128i *hmid, long m, char *nonce)
{
  int i = 0, n = 0, j = 0;

  __m128i lcpy[STATE_LENGTH], hcpy[STATE_LENGTH];
  for (i = 0; !incr(lmid, hmid); i++)
  {
    for (j = 0; j < STATE_LENGTH; j++)
    {
      lcpy[j] = lmid[j];
      hcpy[j] = hmid[j];
    }
    transform64(lcpy, hcpy);
    if ((n = check(lcpy, hcpy, m)) >= 0)
    {
      seri(lmid, hmid, n, nonce);
      return i;
    }
  }
  return -i;
}

// 01:-1 11:0 10:1
void para(char in[], __m128i l[], __m128i h[])
{
  int i = 0;
  for (i = 0; i < STATE_LENGTH; i++)
  {
    switch (in[i])
    {
    case 0:
      l[i] = _mm_set_epi64x(HBITS, HBITS);
      h[i] = _mm_set_epi64x(HBITS, HBITS);
      break;
    case 1:
      l[i] = _mm_set_epi64x(LBITS, LBITS);
      h[i] = _mm_set_epi64x(HBITS, HBITS);
      break;
    case -1:
      l[i] = _mm_set_epi64x(HBITS, HBITS);
      h[i] = _mm_set_epi64x(LBITS, LBITS);
      break;
    }
  }
}

int pwork(char tx[], long mwm, char nonce[])
{
  char trits[TX_LENGTH * 3] = {0}, mid[STATE_LENGTH] = {0};
  __m128i lmid[STATE_LENGTH], hmid[STATE_LENGTH];

  tx2trit(tx, trits);
  absorb(mid, trits, TX_LENGTH * 3 - HASH_LENGTH);

  para(mid, lmid, hmid);
  lmid[0] = _mm_set_epi64x(LOW00, LOW01);
  hmid[0] = _mm_set_epi64x(HIGH00, HIGH01);
  lmid[1] = _mm_set_epi64x(LOW10, LOW11);
  hmid[1] = _mm_set_epi64x(HIGH10, HIGH11);
  lmid[2] = _mm_set_epi64x(LOW20, LOW21);
  hmid[2] = _mm_set_epi64x(HIGH20, HIGH21);
  lmid[3] = _mm_set_epi64x(LOW30, LOW31);
  hmid[3] = _mm_set_epi64x(HIGH30, HIGH31);
  lmid[4] = _mm_set_epi64x(LOW40, LOW41);
  hmid[4] = _mm_set_epi64x(HIGH40, HIGH41);

  return loop_cpu(lmid, hmid, mwm, nonce);
}

void test()
{
  char trits[TX_LENGTH * 3] = {0}, hash[HASH_LENGTH / 3 + 1] = {0},
                         mid[STATE_LENGTH] = {0}, nonce[HASH_LENGTH] = {0};
  char tx[] = "999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999A9RGRKVGWMWMKOLVMDFWJUHNUNYWZTJADGGPZGXNLERLXYWJE9WQHWWBMCPZMVVMJUMWWBLZLNMLDCGDJ999999999999999999999999999999999999999999999999999999YGYQIVD99999999999999999999TXEFLKNPJRBYZPORHZU9CEMFIFVVQBUSTDGSJCZMBTZCDTTJVUFPTCCVHHORPMGCURKTH9VGJIXUQJVHK999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999";

  tx2trit(tx, trits);
  absorb(mid, trits, TX_LENGTH * 3);
  hash2tryte(mid, hash);
  if (strcmp(hash, "IPQYUNLDGKCLJVEJGVVISSQYVDJJWOXCW9RZXIDFKMBXDVZDXFBZNZJKBSTIMBKAXHFTGETEIPTZGNTJK"))
  {
    fprintf(stderr, "hash not match %s\n", hash);
    exit(1);
  }

  long m = 15;
  struct timespec ts, tn;
  timespec_get(&ts, TIME_UTC);
  int cnt = pwork(tx, m, nonce);
  timespec_get(&tn, TIME_UTC);
  long dif = ((long)(tn.tv_sec - ts.tv_sec)) * 1e9 + (tn.tv_nsec - ts.tv_nsec);
  printf("count=%d kHash/sec: %d \n", cnt * 128, (int)((cnt * 128 / 1e3) / (dif / 1e9)));

  memcpy(trits + TX_LENGTH * 3 - HASH_LENGTH, nonce, HASH_LENGTH);
  memset(mid, 0, STATE_LENGTH);
  absorb(mid, trits, TX_LENGTH * 3);
  hash2tryte(mid, hash);
  fprintf(stderr, "PoWed hash is %s\n", hash);

  if (strcmp(hash + HASH_LENGTH / 3 - m / 3, "99999"))
  {
    fprintf(stderr, "PoW is incorrect. %s\n", hash);
    exit(1);
  }
  fprintf(stderr, "OK\n");
}

int main(int argc, char *argv[])
{
  test();
}