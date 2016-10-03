Apache Server LFS Module
========

### Features
* base implement the git lfs server


### Install

* Download the binary (or compile for your self) from [latest release]
* copy `mod_git_lfs.so` to httpd modules
* copy `httpd-git.conf(not authz)` to httpd conf.d and edit

```
#load modeule
LoadModule git_lfs_module modules/mod_git_lfs.so
#the git repo configure
<VirtualHost *:80>
	ServerName loc.g
	SetEnv GIT_PROJECT_ROOT H:/git/repo
	SetEnv GIT_HTTP_EXPORT_ALL
	AliasMatch ^/(.*/objects/[0-9a-f]{2}/[0-9a-f]{38})$  H:/git/repo/$1
	AliasMatch ^/(.*/objects/pack/pack-[0-9a-f]{40}.(pack|idx))$ H:/git/repo/$1
	ScriptAlias / "C:/Program Files/Git/mingw64/libexec/git-core/git-http-backend.exe/"
</VirtualHost>
#the git lfs configure
<VirtualHost *:80>
	ServerName lfs.loc.g
	<Location />
		SetHandler GitLfs
		GitLfsEnabled on
		GitLfsLog 0
		GitLfsHref http://lfs.loc.g/ /
		GitLfsRoot H:/git/lfs/
	</Location>
</VirtualHost>
```
### Options
* `GitLfsEnabled` enable the lfs module
* `GitLfsLog` set the log level
* `GitLfsRoot` the root store path
* `GitLfsHref` the base request url and the prefix to store, it will store file by mapping path `http://lfs.loc.g/<prefix>/<sub1>/<sub2>/objects/<oid>` to  `<root>/<sub1>/<sub2>/oid` 
### Usage
* create repo on server

```
cd /d H:/git/repo
mkdir group
cd group
mkdir test.git
cd test.git
git init --bare
```

* update `group/test.git/conf` to add 

```
[http]
	receivepack = true
```

* clone `git clone http://loc.g/group/test.git`
* adding the all file type to configure `.gitattributes` for using lfs

```
#
# Image
*.jpg filter=lfs diff=lfs merge=lfs -text
*.png filter=lfs diff=lfs merge=lfs -text
*.bmp filter=lfs diff=lfs merge=lfs -text
*.ai filter=lfs diff=lfs merge=lfs -text
*.psd filter=lfs diff=lfs merge=lfs -text
*.gif filter=lfs diff=lfs merge=lfs -text
*.tiff filter=lfs diff=lfs merge=lfs -text
*.tga filter=lfs diff=lfs merge=lfs -text
*.dds filter=lfs diff=lfs merge=lfs -text
*.tif filter=lfs diff=lfs merge=lfs -text
*.ttf filter=lfs diff=lfs merge=lfs -text
```
* adding lfs url configure to `.gitconfig`

```
[lfs]
    url = "http://unset:unset@lfs.loc.g/"

```

* all configure done and enjoy it.

### Compile(win32)
* install visual studio
* download mod_git_lfs source
* download apache binary from <https://httpd.apache.org/>
* download json-c library from <https://github.com/json-c/json-c>
* unzip json-c to `mod_git_lfs/`
* open vs project in `win32`
* update the project include/lib search path for apache
* build it.

### Compile(Linux)
* `yum install httpd-devel`
* download mod_git_lfs source
* download json-c library from <https://github.com/json-c/json-c>
* unzip json-c to `mod_git_lfs/`
* execute blew

```
./autogen.sh
./configure
make
cp -f .libs/mod_git_lfs.so /etc/httpd/modules
```

