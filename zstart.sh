trap test SIGINT &> /dev/null
# PATH
export PODGL=$PWD
export OPT=$PATH
#

# Shell Variables & Built-in Functions

source $PODGL/lib/fsh.cfg.sh
source $PODGL/lib/colors2

initd(){
for i in $(ls $PODGL/etc/init.d);do
   $PODGL/etc/init.d/$i
   if [ "$?" == 1 ];then
      exit 1
   fi
done
}

optionhandler(){
   if [ "$1" == "--std-shell" ] || [ "$1" == "-s" ];then
      export PATH=$PATH:$PODGL/bin/:$PODGL/run/
      source $PODGL/etc/rc.local
      source $PODGL/etc/envir
      source $PODGL/lib/fsh.cfg.sh
      cat $PODGL/etc/motd
      initd
      $SHELL
   fi
   if [ "$1" == "--exec" ] || [ "$1" == "-e" ];then
      export PATH=$PODGL/bin/:$PODGL/run/:$PATH
      source $PODGL/etc/rc.local
      source $PODGL/etc/envir
      source $PODGL/lib/fsh.cfg.sh
      if [ -f "$PODGL/bin/$2" ]; then
	 "$PODGL/bin/$2"
         exit $?
      fi
      if [ -f "$PODGL/run/$2" ]; then
         "$PODGL/run/$2"
         exit $?
      fi
      $2
      exit $?
   fi
}
if [ "${1:0:1}" == "-" ];then
   optionhandler $1 "$2"
   exit 0
fi

if [ "$1" != ''  ];then
    cd $CCWD
    chmod +x $1
    PATH=$PATH:$PODGL/bin:$PODGL/run
    . $1
    exit
    PATH=$OPT
fi


# motd # Message Of The Day
cat etc/motd

# History
HISTFILE=$PODGL/var/digle_history
#set -o history

#cd - &> /dev/null
#

# RC Script
source etc/rc.local

# INIT.D scripts
initd
# Aliases
alias   ls="ls --color"
alias grep="grep --color"

trap test SIGINT &> /dev/null

# SHELL
tcheck(){
if [ -d "$1" ];then
     cd $1
     ./main*
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


#history -c
source $PODGL/etc/envir
while [ 1 ]; do
 if [ "$RANDOM_PROMPT" == "1" ];then
    random prompt
 fi
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
              cerr "$cmd: Command Not Found"
              history -s $cmd
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

#history -c
