#pragma once

#ifdef DEBUG

#define debugs(s)  printf("%s\n", s);
#define debug1(x)  printf("%u\n", num);
#define debug(data,len) { printf(">"); for (int i=0; i<len; i++)  printf("%02X", (unsigned char)*(((unsigned char*)data)+i)); printf("\n");}
  
#else

#define debugs(s)
#define debug1(x)
#define debug(x,y)

#endif
