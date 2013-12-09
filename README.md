# privio... 

A tool for providing privileged file access to a trusted user.  This allows for things like web applications who's users map to POSIX users to manipulate files and directories beloging to said user.  

An example would be a web-based file manager in PHP or Rails or your framework of choice, where a user can login via their LDAP, SSO, etc. credentials and modify their own files via the web interface.

The goal of privio is to be as secure as possible while providing the flexibility to implement the necessary file operations to enable robust web applications that interface with POSIX user files.

## Usage

privio should be setuid root (like sudo) and placed in an accessible path.  I will be implementing upper-level APIs for frameworks like Rails & Jango and possibly a PHP library.

The examples below assume that a user, let's say 'rails', will be calling privio and switching between users for lower-level file access.

Make sure /etc/privio.conf is configured appropriately to allow 'rails' in the 'allowed_users' list.  Also, make sure your secret key is set appropriately in /etc/privio.conf.

As your web app user (say, 'rails')

    [rails@host ~]$ privio ThisIsADefaultSecretKeyAndShouldBeChanged jdoe write /home/jdoe/new_file < some_file

This allowed the rails user to write the contents of 'some_file' to /home/jdoe/new_file, as the user 'jdoe'.
