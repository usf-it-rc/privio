= privio... 

A library for providing privileged file access to a trusted user.  This
allows for things like web applications who's users map to POSIX users
to manipulate files and directories beloging to said user.  

An example would be a web-based file manager in PHP or Rails or your
framework of choice, where a user can login via their LDAP, SSO, etc. 
credentials and modify their own files via the web interface.

The goal of privio is to be as secure as possible while providing the
flexibility to implement the necessary file operations to enable
robust web applications that interface with POSIX user files.
