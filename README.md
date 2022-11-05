# Bonek Root User (boru) <!-- ratio -->

Boru is a minimalist tool to run commands with elevated privileges. A simple alternative to [Sudo](https://www.sudo.ws/sudo/) and [OpenDoas](https://github.com/Duncaen/OpenDoas).


<!--## Installation

If you are on Arch Linux, you can download the package from the AUR. // Didn't upload the pkgbuild to AUR yet.. //
You can install Boru on other distributions but you have to clone and build the package yourself.
-->
### Manual Installation

```
$ git clone https://github.com/zKBTuran/boru
$ cd boru
# make
# sudo make install
```
After that, you are basically done. **If it doesn't work, you might have to change the configuration file. Which is in `/etc/boru.conf`, and set the group variable to the admin group you are in. If you don't know which group is it, you can check out `/etc/group`.**

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
