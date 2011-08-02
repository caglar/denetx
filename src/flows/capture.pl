#!/usr/bin/perl -ws
use strict ;
use warnings;

use lib("./");
use Net::CFlow;
use IO::Socket::INET;
use Data::Dumper;

sub flow_ver() {9}              # 9 or 10
sub SetId() { flow_ver() == 9 ? 0 : 2 }

my $receive_port = 8991 ;
my $packet = undef ;

=comment
my $MTemplateRef =
{
    SetId        => SetId,
    TemplateId   =>300,
    Template => [
        { Length => 4, Id => 8  }, # SRC_ADDR
        { Length => 4, Id => 12 }, # DST_ADDR
        { Length => 4, Id => 2  }, # PKTS
        { Length => 4, Id => 1  }, # BYTES
        { Length => 2, Id => 7  }, # SRC_PORT
        { Length => 2, Id => 11 }, # DST_PORT
        { Length => 1, Id => 4  }, # PROT
        { Length => 1, Id => 5  }, # TOS
        { Length => 4, Id => 34 }, #SAMPLING_INT
        { Length => 4, Id => 35 }, # SAMPLING_ALG
    ],
} ;

my @TemplateArrayRef = ( $MTemplateRef ) ;
warn Dumper(\@TemplateArrayRef), "\n";
=cut
=comment
my $TemplateArrayRef =
{
    SetId        => SetId,
    TemplateId   =>300,
    Template => [
        { Length => 4, Id => 8  }, # SRC_ADDR
        { Length => 4, Id => 12 }, # DST_ADDR
        { Length => 4, Id => 2  }, # PKTS
        { Length => 4, Id => 1  }, # BYTES
        { Length => 2, Id => 7  }, # SRC_PORT
        { Length => 2, Id => 11 }, # DST_PORT
        { Length => 1, Id => 4  }, # PROT
        { Length => 1, Id => 5  }, # TOS
        { Length => 4, Id => 34 }, #SAMPLING_INT
        { Length => 4, Id => 35 }, # SAMPLING_ALG
    ],
};
=cut

my $TemplateArrayRef = [
    { Length => 4, Id => 8  }, # SRC_ADDR
    { Length => 4, Id => 12 }, # DST_ADDR
    { Length => 4, Id => 2  }, # PKTS
    { Length => 4, Id => 1  }, # BYTES
    { Length => 2, Id => 7  }, # SRC_PORT
    { Length => 2, Id => 11 }, # DST_PORT
    { Length => 1, Id => 4  }, # PROT
    { Length => 1, Id => 5  }, # TOS
    { Length => 4, Id => 34 }, #SAMPLING_INT
    { Length => 4, Id => 35 }, # SAMPLING_ALG
];

my $sock = IO::Socket::INET->new( LocalPort =>$receive_port, Proto => 'udp') ;

while ($sock->recv($packet,1548)) {
    my ($HeaderHashRef,$FlowArrayRef,$ErrorsArrayRef) = () ;
    ($HeaderHashRef,
        $TemplateArrayRef,
        $FlowArrayRef,
        $ErrorsArrayRef) = Net::CFlow::decode(\$packet, $TemplateArrayRef);

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

print "My Flow\n";
print Dumper(@{$FlowArrayRef});

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
