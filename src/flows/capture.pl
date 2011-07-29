#!/usr/bin/perl -ws
use strict ;
use warnings;
use Net::Flow qw(decode) ;
use IO::Socket::INET;

my $receive_port = 9993 ;
my $packet = undef ;
my $TemplateArrayRef = undef ;
my $sock = IO::Socket::INET->new( LocalPort =>$receive_port, Proto => 'udp') ;

while ($sock->recv($packet,1548)) {

    my ($HeaderHashRef,$FlowArrayRef,$ErrorsArrayRef)=() ;

    ( $HeaderHashRef,
        $TemplateArrayRef,
        $FlowArrayRef,
        $ErrorsArrayRef)
    = Net::Flow::decode(
        \$packet,
    $TemplateArrayRef
) ;

grep{ print "$_\n" }@{$ErrorsArrayRef} if( @{$ErrorsArrayRef} ) ;

print "\n- Header Information -\n" ;
foreach my $Key ( sort keys %{$HeaderHashRef} ){
    printf " %s = %3d\n",$Key,$HeaderHashRef->{$Key} ;
}

foreach my $TemplateRef ( @{$TemplateArrayRef} ){
    print "\n-- Template Information --\n" ;

    foreach my $TempKey ( sort keys %{$TemplateRef} ){
        if( $TempKey eq "Template" ){
            printf "  %s = \n",$TempKey ;
            foreach my $Ref ( @{$TemplateRef->{Template}}  ){
                foreach my $Key ( keys %{$Ref} ){
                    printf "   %s=%s", $Key, $Ref->{$Key} ;
                }
                print "\n" ;
            }
        }else{
            printf "  %s = %s\n", $TempKey, $TemplateRef->{$TempKey} ;
        }
    }
}

foreach my $FlowRef ( @{$FlowArrayRef} ){
    print "\n-- Flow Information --\n" ;

    foreach my $Id ( sort keys %{$FlowRef} ){
        if( $Id eq "SetId" ){
            print "  $Id=$FlowRef->{$Id}\n" if defined $FlowRef->{$Id} ;
        }elsif( ref $FlowRef->{$Id} ){
            printf "  Id=%s Value=",$Id ;
            foreach my $Value ( @{$FlowRef->{$Id}} ){
                printf "%s,",unpack("H*",$Value) ;
            }
            print "\n" ;
        }else{
            printf "  Id=%s Value=%s\n",$Id,unpack("H*",$FlowRef->{$Id}) ;
        }
    }
}
}
1;
