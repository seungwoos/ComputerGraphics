#include <stdio.h>
#include <string.h>

int main() {
  FILE *fp = fopen("13517_Beach_Ball_v2_L3.obj", "r") ;
  char buffer[255] ;

  if (fp==NULL) {
    printf("no file\n") ;
    return -1 ;
  }

  float max_x=-10.0, max_y=-10.0, max_z=-10.0 ;
  float min_x=10.0, min_y=10.0, min_z=10.0 ;
  while (fgets(buffer, 255, fp)) {
    if (buffer[0] == 'v' && buffer[1]==' ') {
      float x, y, z ;
      sscanf(buffer, "v %f %f %f", &x, &y, &z) ;
      max_x = max_x > x ? max_x : x ;
      max_y = max_y > y ? max_y : y ;
      max_z = max_z > z ? max_z : z ;

      min_x = min_x < x ? min_x : x ;
      min_y = min_y < y ? min_y : y ;
      min_z = min_z < z ? min_z : z ;
      //printf("%f\n", y) ;
    }
  }
  fclose(fp) ;
  printf("%f %f\n", min_x, max_x) ;
  printf("%f %f\n", min_y, max_y) ;
  printf("%f %f\n", min_z, max_z) ;
}
