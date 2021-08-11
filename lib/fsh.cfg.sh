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
printf "┌$white──[$purple$PWD$white]----[$blue$(printf $VICTIM | sed s/" "//g)$white]-[$red$PORT$white]\n$white▼$white\n$green"
read -e $1
printf "$blyellow↓$green\n"
}

rlog(){
if [ -f "var/log/slog.bgl" ];then
 cout "Removing var/log/* ... !"
 sleep 0.3
 rm $PODGL/var/log/* $PODGL/var/digle_history &> /dev/null
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
say(){
  timeout 5 morse -f 400 a
  espeak -v en-us "\"$@\""
  timeout 5 morse -f 400 n
  killall -9 morse &> /dev/null
}
