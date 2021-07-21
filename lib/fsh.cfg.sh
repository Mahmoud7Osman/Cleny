source $PODGL/lib/colors

# Output
cout(){
printf $green"☢$white %s\n" "$*"
}
# Error
cerr(){
printf $red"☢$white %s\n" "$*"
}
# logger
clog(){
printf "$green`whoami`$white @ `date` $yellow%s\n" "$*" >> $PODGL/var/log/slog.bgl
}
# Check Box Yes
cby(){
printf $white"["$green" ✔ "$white"]$white %s\n" "$*"
}
# Check Box No
cbn(){
printf $white"["$red" ✘ "$white"]$white %s\n" "$*"
}

cin(){
printf "┌$white──[$cyan$PWD$white]----[$blgreen$(printf $VICTIM | sed s/" "//g)$white]-[$blpurple$PORT$white]\n▼\n$mag"
read -e $1
sleep 0.05
printf "$green↓$yellow\n"
sleep 0.05
}

rlog(){
if [ -f "var/log/slog.bgl" ];then
 cout "Removing var/log/* ... !"
 sleep 0.3
 rm $PODGL/var/log/* $PODGL/var/digle_history
else cerr "No Logs Found"
fi
}

victim(){
  case $1 in
  ip) export VICTIM=$2;;
  port)export PORT=$2;;
  *) cerr Invalid Usage;;
  esac
}
prepare(){
   cout "Preparing Automated Values from $PODGL/etc/auto.dgl"
   source $PODGL/etc/auto.dgl
}

lset(){
 if [ "$1" == '' ] || [ "$2" == '' ];then
    cerr No Enough Arguments, see info lset

 else
 export ${1^^}=$2 2>/dev/null
 printf "${green}$1${cyan} => ${yellow}$2\n"
 fi
}
use(){
if [ ! -d "$PODGL/sys/$1" ];then
   cerr "Error: Cannot Use Module '$1', Module not found"
elif [ "$1" == '' ];then
   printf "${green}use$yellow malware\n${green}use$yellow bombs\n"
else
export WHAT=$1
fi
}
prompt(){
if [ "$1" == '' ] || [ ! -f "$PODGL/etc/clins/$1" ];then
   cerr "Error!, Execute 'show prompts'"
else
source $PODGL/etc/clins/$1
fi
}
