// PE0FKO
//

#define DV    24        // Default Volume
static const struct _stationList {
  const char    *name;  // Noame of the stream
  const char    *url;   // URL of stream server
  unsigned int  volume; // The default start volume
  bool          mp3;    // mp3 or AAC
} stationList[] = 
{
  // http://pe1chl.nl/repeaters/
  { "PI2NOS 430.125"  , "http://stream.hobbyscoop.nl/pi2nos"              , DV+30, true },   // 00
  { "PI3UTR 145.275"  , "http://stream.hobbyscoop.nl/pi3utr"              , DV+30, true },   // 01
  { "PI6NOS 1298.375" , "http://stream.hobbyscoop.nl/pi6nos"              , DV+30, true },   // 02

//  { "PI2NON 430.275"  , "http://pc7x.net/audio/pi2non/"                   , DV+20, true },   // 03
  { "PI2NON 430.275"  , 
                        "http://pa0ebc.nl:9744/broadwave.mp3?src=2&rate=0", DV+30, true },   // 03


  { "PI3GOE 145.755"  , "http://stream.hobbyscoop.nl/pi3goe"              , DV, true },   // 04
  { "PI1DFT 430.087"  , "http://stream.hobbyscoop.nl/pi1dft"              , DV, true },   // 05
  { "PI1ZLD 438.400"  , "http://stream.hobbyscoop.nl/pi1zld"              , DV, true },   // 06

  { "PI3HVN 145.700"  , "http://44.137.68.19:9745/broadwave.mp3?src=1&rate=1" , DV, true },  // Heerenveen
  { "PI3HVN 430.025"  , "http://44.137.68.19:9745/broadwave.mp3?src=2&rate=1" , DV, true },  // Heerenveen

  // https://www.hendrikjansen.nl/henk/streaming.html
  //  -sb-  96 Kbps   12 KB/s
  //  -bb-  192 Kbps  24 KB/s

// http://icecast.omroep.nl/radio5-sb-aac
//  { "NPO Radio 5"     , "http://icecast.omroep.nl/radio5-sb-mp3"          },
// http://www.mp3streams.nl/zender/npo-radio-5/stream/6-aac-64

  { "NPO Radio 1"     , "http://icecast.omroep.nl/radio1-sb-mp3"          , DV, true },     // 09
  { "NPO Radio 2"     , "http://icecast.omroep.nl/radio2-sb-mp3"          , DV, true },
  { "NPO Radio 3"     , "http://icecast.omroep.nl/3fm-sb-mp3"             , DV, true },
  { "NPO Radio 4"     , "http://icecast.omroep.nl/radio4-sb-mp3"          , DV, true },
  { "NPO Radio 5"     , "http://icecast.omroep.nl/radio5-sb-mp3"          , DV, true },     // 13
  { "NPO Radio 6"     , "http://icecast.omroep.nl/radio6-sb-mp3"          , DV, true },
  { "Radio Gelderland", "http://stream.omroepgelderland.nl/radiogelderland/pls.php" , DV, true },
  { "Radio 10"         , "http://stream.radio10.nl/radio10"               , DV, true },
  //{ "Nieuws NOS"      , "http://www.internettuner.nl/nos/nieuws"          , DV, true },
  //{ "BNR Nieuws"      , "http://icecast-bnr.cdp.triple-it.nl/bnr_mp3_32_03" , DV, true },

  { "NPO Radio 1 ACC" , "http://icecast.omroep.nl/radio1-sb-aac"          , DV, false },    // 17
  { "NPO Radio 2 ACC" , "http://icecast.omroep.nl/radio2-sb-aac"          , DV, false },
  { "NPO Radio 3 ACC" , "http://icecast.omroep.nl/3fm-sb-aac"             , DV, false },
  { "NPO Radio 4 ACC" , "http://icecast.omroep.nl/radio4-sb-aac"          , DV, false },
  { "NPO Radio 5 ACC" , "http://icecast.omroep.nl/radio5-bb-aac"          , DV, false },    // 21
  { "NPO Radio 6 ACC" , "http://icecast.omroep.nl/radio6-sb-aac"          , DV, false },

  { "NashvilleFM"     , "http://nashvillefm.stream-server.nl:8300/stream" , DV, true },

  { "Asia Dream"      , "https://igor.torontocast.com:1025/;.mp3"         , DV, true },
  { "KPop Radio"      , "http://streamer.radio.co/s06b196587/listen"      , DV, true },
  { "Classic FM"      , "http://media-ice.musicradio.com:80/ClassicFMMP3" , DV, true },
  { "Lite Favorites"  , "http://naxos.cdnstream.com:80/1255_128"          , DV, true },
  { "MAXXED Out"      , "http://149.56.195.94:8015/steam"                 , DV, true },
  { "SomaFM Xmas"     , "http://ice2.somafm.com/christmas-128-mp3"        , DV, true },
  { 0, 0, 0, 0}
};

const   int   stationListNumber   = sizeof(stationList) / sizeof(stationList[0]) - 1;
const   int   StationListDefault  = 13;
