#include <cstdlib>

int main(void){
  system("/bin/bash -c 'source $PODGL/lib/fsh.cfg.sh; cout Press Control-C To Exit;echo; cat >/dev/null'");
}
