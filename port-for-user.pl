#! /usr/bin/perl -w
use strict;
use Digest::MD5;
#
# port-for-user.pl - Return a port number, p, for a given user, with a
#     low probability of collisions. The port p is always even, so that
#     users can use p and p+1 for testing with proxy and the Tiny web
#     server.
#     
# 이 파일 사용해서 proxy나 tiny server에 대한 고유 포트를 생성
#
# Generates a random port for a particular user usage: ./port-for-user.pl <userID>
#     usage: ./port-for-user.pl [optional user name]
#
my $maxport = 65536;
my $minport = 1024;


# hashname - compute an even port number from a hash of the argument
sub hashname {
    my $name = shift;
    my $port;
    my $hash = Digest::MD5::md5_hex($name);
    # take only the last 32 bits => last 8 hex digits
    $hash = substr($hash, -8);
    $hash = hex($hash);
    $port = $hash % ($maxport - $minport) + $minport;
    $port = $port & 0xfffffffe;
    print "$name: $port\n";
}


# If called with no command line arg, then hash the userid, otherwise
# hash the command line argument(s).
# hash: 특정 데이터를 이를 상징하는 더 짧은 길이의 데이터로 변환하는 행위.
if($#ARGV == -1) {
    my ($username) = getpwuid($<);
    hashname($username);
} else {
    foreach(@ARGV) {
        hashname($_);
    }
}
