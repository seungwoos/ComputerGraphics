#include <stdio.h>
#include <string.h>

int main() {
  FILE *fp = fopen("13517_Beach_Ball_v2_L3.obj", "r") ;
  FILE *wp = fopen("Beach_ball.obj", "w") ;
  char buffer[255] ;

  if (fp==NULL || wp ==NULL) {
    printf("no file\n") ;
    return -1 ;
  }

  while (fgets(buffer, 255, fp)) {
    if (buffer[0] == 'v' && buffer[1]==' ') {
      float x, y, z ;
      sscanf(buffer, "v %f %f %f", &x, &y, &z) ;
      fprintf(wp, "v %f %f %f\n", x, y, z-15.48) ;
    } else if (buffer[0]=='v' && buffer[1]=='1') {
      float x, y, z ;
      sscanf(buffer, "vn %f %f %f", &x, &y, &z) ;
      fprintf(wp, "vn %f %f %f\n", x/10, y/10, z/10) ;
    } else {
      fwrite(buffer, 1, strlen(buffer), wp) ;
    }
  }
  fclose(fp) ;
  fclose(wp) ;
}
