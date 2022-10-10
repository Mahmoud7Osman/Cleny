if [ $(whoami) != "root" ]; then
	printf "ERROR: Sudo Privileges Required To Install Cleny On Your System\n"
	printf "Please Run 'sudo bash install.sh'\n"
	exit 1
fi

g++ base/cleny.cpp -o bin/cleny &> /dev/null

printf " -> https://github.com/Mahmoud7Osman/Cleny\n"
printf "in the README.md section, You will find everything you need\n\n"
printf "Thank You For Installing Cleny\n"

if [ ! -f "bin/cleny" ]; then
	cp bin/x64_cleny /bin/cleny
	chmod uog+x /bin/cleny
	exit 0
fi
cp bin/cleny /bin/cleny
chmod uog+x /bin/cleny
