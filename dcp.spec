Name:		DCP
Version:	master
Release:	1%{?dist}
Summary:	Digest, stat, and copy files from one location to another in the same read pass 

Group:		Applications/Archiving
License:	MIT
URL:		https://github.com/NationalSecurityAgency/DCP
Source0:	https://github.com/NationalSecurityAgency/DCP/archive/master.tar.gz

BuildRequires:	gengetopt autoconf automake libtool jansson-devel openssl-devel libdb-devel	
Requires:	openssl jansson libdb

%description
dcp combines cp, stat, md5sum and shasum to streamline mirroring and gathering
information about all the files copied. All information gathered is written to 
an output file. The output file can be fed back into dcp when copying 
snapshots of a directory, this allows only files which differ in location or 
hash to be copied.

%prep
%setup -q


%build
./bootstrap.sh
%configure
make %{?_smp_mflags}


%install
make install DESTDIR=%{buildroot}


%files
/usr/bin/dcp
%doc
/usr/share/man/man1/dcp.1.gz


%changelog
* Fri Mar 22 2019 Jonas Svatos <jonas.svatos@nfa.cz> master-1
- Initial RPM release

