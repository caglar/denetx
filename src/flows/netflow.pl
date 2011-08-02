use strict;
use Net::Flow qw(decode encode) ;
use IO::Socket::INET ;
use Socket;
use Data::Dumper;

my $receive_port = 8991;
my $send_port    = 9996;

my $ErrorsArrayRef;
my $packet;
my $TemplateRef;

sub flow_ver() {9}              # 9 or 10
sub SetId() { flow_ver() == 9 ? 0 : 2 }

my $MyTemplateRef={
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
    { Length => 4, Id => 34 }, # SAMPLING_INT
    { Length => 4, Id => 35 }, # SAMPLING_ALG
    ],
} ;

my @MyTemplates = ( $MyTemplateRef ) ;
warn Dumper(\@MyTemplates), "\n";

my $EncodeHeaderHashRef = {
  SourceId    => 0,
  VersionNum  => flow_ver,
  SequenceNum => 0,
  ObservationDomainId => 0,
} ;

my $s_sock = IO::Socket::INET->new( PeerAddr => '127.0.0.1',
                    PeerPort =>  $send_port,
                    Proto => 'udp' ) ;

my @flows;
for (0..255){
  #incorporate $_ into a few flows so we can see which flows get encoded.
  push @flows, { SetId=>300,
         8=>inet_aton('10.1.15.'.$_),
         12=>inet_aton('10.1.15.2'), 
         2=>pack('N',10+$_), 
         1=>pack('N',800+$_),
         7=>pack('n',80),
         11=>pack('n',4242),
         4=>pack('C',6),
         5=>pack('C',0),
         34=>pack('N',1),
         35=>pack('N',2), };
}

for (1..1) {
  for my $FlowArrayRef ( [ @flows ] ) {

    my $PktsArrayRef = undef ;

    $EncodeHeaderHashRef->{SysUpTime}    = $^T-time;
    $EncodeHeaderHashRef->{UnixSecs}     = time;

    warn Dumper($EncodeHeaderHashRef), "\n";

    ( $EncodeHeaderHashRef,
      $PktsArrayRef,
      $ErrorsArrayRef)
      = Net::Flow::encode(
      $EncodeHeaderHashRef,
      \@MyTemplates,
      $FlowArrayRef,
      1400,
      );
    warn "=====================================\n";
    warn Dumper($EncodeHeaderHashRef), "\n";

    grep{ print "$_\n" }@{$ErrorsArrayRef} if( @{$ErrorsArrayRef} ) ;

    foreach my $Ref (@{$PktsArrayRef}){
      $s_sock->send($$Ref) ;
    }
  }
}
