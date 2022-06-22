# Bonek Root User (boru) <!-- ratio -->

Boru is a simple alternative for sudo and doas.


## Installation

If you are on Arch Linux, you can download the package from the AUR. // not uploaded the pkgbuild too aur yet //
Or else you are on other distribution you have to clone and build the package yourself.

### Manual Installation

```
$ git clone https://github.com/zKBTuran/boru
$ cd boru
# make
# sudo make install
```
After that, you'll have to configure boru to allow you to use it. To do this, edit /etc/boru.conf, and set the group variable to the admin group you are in.

### Manual Uninstallation

```# make uninstall```

## Usage

```$ boru [command]```

## Configuration

This is the default configuration for boru:

```
group=wheel
wrong_pw_sleep=1000
session_ttl=5
nopass=1
```
Here is the meaning of these variables:

**group**: The group of users that is allowed to execute boru.

**wrong_pw_sleep**: The amount of milliseconds to sleep at a wrong password attempt. Must be a positive integer. Set to 0 to disable.

**session_ttl**: The amount of minutes a session lasts. Must be a positive integer. Set to 0 to disable.

**nopass**: Sets whether the boru will ask for password or not. Set to 0 to enable. ⚠️❗ ***Warning! This may cause a huge security hole in your system. This option is not recommended. Please do not use it if you have important data on your device.*** ❗⚠️ 
