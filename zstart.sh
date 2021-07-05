trap test SIGINT &> /dev/null
# PATH
export PODGL=$PWD

#

# Shell Variables & Built-in Functions

source $PODGL/lib/fsh.cfg.sh
source $PODGL/etc/envir
source $PODGL/lib/colors2

if [ "$1" != '' ];then
    chmod +x $1
    . ./$1
    exit
fi


# motd # Message Of The Day
cat etc/motd

# History
HISTFILE=$PODGL/var/digle_history
set -o history



#cd - &> /dev/null
#

# RC Script
digle etc/rc.local

# INIT.D scripts
for i in $(ls etc/init.d);do
   etc/init.d/$i
done

# Aliases
alias   ls="ls --color"
alias grep="grep --color"

trap test SIGINT &> /dev/null

# SHELL
history -c
tcheck(){
if [ -d "$1" ];then
     cd $1
     ./main.*
     cd - &> /dev/null
     return 0
fi
return 1
}


if [ -f ".update" ];then
   printf "${white}file ($green.update$white) Found !! Running Updater...!\n"
   sleep 0.5
   bash run/update
   rm .update
fi



while [ 1 ]; do
 cin cmd
 if [ "$cmd" == "" ];then
   continue
 fi
 # PATH
 for path in $(/bin/cat $PODGL/etc/PATH.dgl); do
    if [ ! -f "$path$(echo $cmd | head -n1 | awk '{print $1;}')" ];then
      tcheck $path$cmd
      if [ "$?" == 0 ];then continue;fi
      # PATH
      if [ "$path" == "$(/bin/tail -1 $PODGL/etc/PATH.dgl)" ] && [ ! -f "$(/bin/tail -1 $PODGL/etc/PATH.dgl)$cmd" ];then
	 type $(echo $cmd | head -n1 | awk '{print $1;}') &> /dev/null
         if [ "$?" != 0 ];then
              cerr "$red$(echo $cmd | head -n1 | awk '{print $1;}')$white Command Execution Error: Command Not Found"
         else
              $cmd
              history -s $cmd
	      break;
         fi
         break;
      # PATH
      elif [ -f "$(/bin/tail -1 $PODGL/etc/PATH.dgl)$(echo $cmd | head -n1 | awk '{print $1;}')" ];then
	 # PATH
	 $(/bin/tail -1 $PODGL/etc/PATH.dgl)$cmd
         history -s $cmd
	 break;
      else
      continue
      fi
    continue
    fi
    $path$cmd
    history -s $cmd
    break;
 done
done

history -c
