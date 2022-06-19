# boru
Bonek Root User / a simple replacement for rdo sudo and doas.

If you are on Arch Linux, you can download the package via the AUR.
Else if you are on other distrobution you have to clone and bluild the package yourself.

To install:

git clone https://github.com/zKBTuran/boru
cd rdo
make
sudo make install
After that, you'll have to configure rdo to allow you to use it. To do this, edit /etc/boru.conf, and set the group variable to the admin group you are in.

To uninstall:

sudo make uninstall
Usage
boru [command]
The configuration file has the following variables:

group=wheel
wrong_pw_sleep=1000
session_ttl=5
group: The group of users that is allowed to execute boru.
wrong_pw_sleep: The amount of milliseconds to sleep at a wrong password attempt. Must be a positive integer. Set to 0 to disable.
session_ttl: The amount of minutes a session lasts. Must be a positive integer. Set to 0 to disa
