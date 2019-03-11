
#ifndef STREAMUNLIMITED_GPA_H
#define STREAMUNLIMITED_GPA_H

#include <string.h>
//#include <jni.h>
#include <stdio.h>
#include <pthread.h>
//#include <malloc.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <csignal>
#include <sys/socket.h>
#include <net/if.h>
#include <poll.h>

#include "md5.h"
#include "types.h"
#include "mongoose.h"
#include <miniupnpc.h>

#define bDisabletimecheckforDebug

#ifdef GOALBIT_ANDROID
#define _CLOCK_REALTIME 0
#endif

struct xmlst {
    char* p0;
    char* p4;
    char* p8;
    int nC;
    char* p10;
    char* (*f14)(char*, char*, int);
    int (*f18)(char*, char*, int);
    char* (*f1C)(char*, char*, int);
    void (*f20)(char*, char *, int, char *, int);
} ;

typedef struct piece_request_info_t_0 {
    int i_stream;
    int i_piece;
    int i_segment;
    int64_t i_offset;
    int64_t i_size;
} piece_request_info_t;

struct INCOMING_CONNECTION {
    sockaddr_in addr;
    int sk;
};

union value_t {
    int i_int;
    bool b_bool;
    float_t f_float;
    char*  psz_string;//1
    int64_t i_time;
    piece_request_info_t* piece_request_info;
    INCOMING_CONNECTION* incoming_connection;
};

struct block_t {
    int i_buffer_size;
    int i_buffer_index;
    uint8_t* p_buffer;
};

typedef struct _cache_piece {
    int i_piece_id;
    int i_piece_quality;
    block_t* p_block;
    _cache_piece* p_newer;
    _cache_piece* p_older;
    bool b_flushed;
} CachePiece;

enum session_t {
    NO_SESSION_TYPE,
    HTTP_PROGRESSIVE_DOWNLOAD,
    HTTP_LIVE_STREAMING
};

class GoalBitPiece {
public:
    GoalBitPiece(int _piece_id, uint8_t *_piece_content, int _piece_size, float _piece_duration, bool _piece_discont);
    int GetId();
    int GetSize();
    void GetContent(uint8_t **content);
    double GetDuration();
    bool GetDiscont();

    int piece_id;
    uint8_t* piece_content;
    int piece_size;
    float_t piece_duration;
    bool piece_discont;
};

struct goalbit_segment_info_t_0 {
    int i_quality;
    float f_duration;
    bool b_discontinuity;
};

struct goalbit_structure_t;
struct goalbit_param_t_0;

typedef struct goalbit_t_0 {
    goalbit_structure_t* p_structure;
    goalbit_param_t_0* p_param;
    pthread_t* p_client;
    pthread_mutex_t client_lock;
} goalbit_t;

class btBitField {
public:
    void _read_bitfield(size_t i_row, char *psz, size_t i_off, size_t i_length);
    char* GetBitfieldData(size_t i_offset, size_t i_length, size_t *i_size);
    void print();
    int SelectRandomUniformID(size_t i_row);
    bool IsEmpty(size_t i_row);
    void UnsetBiggerPieces(size_t idx);
    void UnSet(size_t idx);
    void UnsetSmallerPieces(size_t idx);
    void Xor(const btBitField *bf);
    btBitField(btBitField* bf);
    void Set(size_t i_row, size_t idx);
    btBitField(goalbit_t_0 *p_goalbit, size_t i_row_count, bool b_hls_win);
    void UnSet(size_t i_row, size_t idx);
    void RemoteUpdateWindow(size_t i_new_offset);
    void UpdateWindow(size_t i_new_offset);
    bool IsSet(size_t i_row, size_t idx);//whether content is already written as file

    goalbit_t_0* p_this;
    int i_min_id;//win_offset
    int i_max_id;//win_offset+win_length - 1
    int nbits;//win_length
    int nbytes;//nbits >> 3
    int i_rows;//stream number
    bool b_hls_window;//hls or p2p
    unsigned char** b;//char array(nbytes), array number = i_rows,
    size_t* nset;//pointer array, array number = i_rows
    void SetBitfieldData(char *string, size_t i, size_t i1);

    void _write_bitfield(size_t v35, char *buffer, size_t offset, size_t v8);
};

class btSelfBitField : public btBitField {
public:
    btSelfBitField(btBitField *bf);

    void UnsetPiece(size_t i_row, size_t idx);
    void SetPiece(size_t i_row, size_t idx);
    int IsPieceSet(size_t i_row, size_t idx);

    //btBitField baseclass_0;
};

typedef int64_t mtime_t;

class MpegTS {
public:
    void parsePAT(unsigned char *p_pat_content);
    MpegTS(goalbit_t_0 *p_goalbit);
    double_t parseContent(block_t *p_block);
    static unsigned int getDuration(block_t *p_block, int i_offset, int i_length);

    goalbit_t_0* p_this;
    u_char * p_pat;
    u_char * p_pmt;
    int i_pmt_pid;
};

class btFiles {
public:
    size_t get_file_size(FILE *p_file);
    int exists_segment_file(size_t i_segment_quality, size_t i_segment_id);
    FILE* create_file(size_t i_segment_quality, size_t i_segment_id, const char *psz_options);
    unsigned int writeSegment(size_t i_segment_quality, size_t i_segment_id, block_t *p_segment_data);
    char *get_file_absolute_path(int i_segment_quality, int i_segment_id);
    void window_changed();
    btFiles(goalbit_t_0 *p_goalbit);
    int writePiece(size_t i_piece_quality, size_t i_piece_id, block_t *p_piece_data);
    block_t* readSegment(size_t i_segment_quality, size_t i_segment_id);
    block_t* readPiece(size_t i_piece_quality, size_t i_piece_id);

    goalbit_t_0 *p_this;
    void* p_pieces;
    int i_segment_offset;//i_hls_win_offset
};

class btCache {
public:
    void FlushOldest();
    int FlushNeeded();
    block_t* GetPiece(size_t i_piece_quality, size_t i_piece_id);
    int AddPiece(block_t *p_data, size_t i_piece_quality, size_t i_piece_id, bool b_should_write);
    int WriteSegment(size_t i_quality, size_t i_segment_id, block_t *p_data);
    size_t RemovePiece(CachePiece *p_piece, bool b_flush);
    void UpdateWindow();
    btCache(goalbit_t *p_goalbit);
    void FlushAll();
    void FlushPiece(CachePiece *p_piece);
    block_t* ReadSegment(size_t i_quality, size_t i_segment_id);

    goalbit_t* p_this;
    int i_cache_used;
    int i_cache_size;//p_param->i_cache_size
    CachePiece** p_pieces;
    CachePiece* p_oldest;
    CachePiece* p_newest;
    CachePiece* p_toflush;
    btFiles* p_files;
    int i_unflushed;
    int i_piece_offset;//i_p2p_win_offset
    bool b_flush_failed;
};

struct pair_value_t {
    value_t value1;
    value_t value2;
};

class btContent {
public:
    int GetOptimisticDownInterval();
    void UpdateHLSStats(size_t i_quality, size_t i_segment_id);
    int GetABIMaxPiece();
    unsigned int GetPieceSize(size_t i_piece_quality, size_t i_piece_id);
    void CheckP2PBitFieldFromHLS(btBitField *bf);
    void UpdateP2PWindow(size_t i_new_p2p_offset);
    void SyncWindows();
    array_t<metadata_piece_t>* GetMissingPiecesInSegment(size_t i_quality, size_t i_segment_id);
    void ExecutionMonitor();
    void UpdateP2PStats(size_t i_quality, size_t i_segment_id, size_t i_piece_id);
    long double getPlayableDurationFromSegment(size_t i_segment_id);
    void update_ABD(bool b_new_content);
    void checkSegmentCompletion(size_t i_piece_quality, size_t i_segment_id, size_t i_piece_id);
    unsigned int WritePiece(size_t i_piece_quality, size_t i_piece_id, block_t *p_piece_data, bool b_safe_content);
    void ProcessDownloadedHLSPieces();
    int WriteSegment(size_t i_quality, size_t i_segment_id, block_t *p_segment_data);
    void ProcessDownloadedHLSSegments();
    void InitP2PExecution();
    void InitHLSExecution();
    int InitP2PInfo();
    int InitHLSBitfield();
    unsigned int InitP2PBitfield();
    void BitFieldP2PUpdate();
    void WindowCheck();
    void UpdateHLSWindow(size_t i_new_hls_offset);
    btContent(goalbit_t_0 *p_goalbit);
    btContent(const btContent &goalbit_t_0);
    block_t* VideoPlayerPreBuffer(int *pi_status, goalbit_segment_info_t_0 *p_info);
    block_t* ReadSegmentContent(size_t i_quality, size_t i_segment_id, size_t i_start_offset, size_t i_length, bool *pb_completed);
    block_t* ReadSegment(size_t i_quality, size_t i_segment_id);
    block_t* ReadPiece(size_t i_piece_quality, size_t i_piece_id);
    block_t * VideoPlayerRead(int *pi_status, goalbit_segment_info_t_0 *p_info);

    goalbit_t_0 *p_this;
    int fd_status;
    MpegTS* p_mpegts;
    btCache* p_cache;
    btSelfBitField* p_p2p_bitfield;
    btSelfBitField* p_hls_bitfield;
    btSelfBitField* p_hls_discontinuity;
    btSelfBitField* p_hls_error;
    u_char m_shake_buffer[65];
    time_t m_start_timestamp;
    int64_t i_last_abi;
    int64_t i_last_abi_dup;
    int64_t i_last_recv;
    int64_t i_last_opt_download;
    int i_last_quality_down;
    int i_same_quality_count;
    bool b_prebuffer;
};

class btBuffer {
public:
    btBuffer() {
        this->b_socket_closed = 0;
        this->i_buffer_count = 0;
        this->i_buffer_size = 0x4B000;
        this->p_buffer_data = (char *)malloc(0x4B000u);
    }

    ~btBuffer() {
        if ( this->p_buffer_data )
            delete this->p_buffer_data;
    }
    void Close();
    void Reset();
    void Remove(size_t *const i_length);
    ssize_t PutAndFlush(SOCKET sk, char *p_data, size_t i_length);
    unsigned int _SEND(SOCKET sk, char *p_data, size_t i_length);
    ssize_t Flush(SOCKET sk);
    int Put(SOCKET sk, char *p_data, size_t i_length);
    int _RECV(SOCKET sk, char *p_data, size_t i_length);

    bool _increase_buffer();
    ssize_t Feed(SOCKET sk);
    char* Get(size_t *const i_length);

    char* p_buffer_data;
    int i_buffer_count;
    int i_buffer_size;
    bool b_socket_closed;
};

class btStream {
public:
    size_t RemoveMessage();
    ssize_t Send_Keepalive();
    btStream(goalbit_t_0 *pT_0);
    void SetSocket(SOCKET sk){ sock = sk; }
    size_t GetInBufferLeftSize();
    bool CanIncreaseInBuffer();
    unsigned int Send_WinUpdate(size_t i_piece_id);
    unsigned int Send_Have(size_t i_piece_quality, size_t i_piece_id, size_t i_abi, size_t i_peer_quality);
    ssize_t Send_Handshake(char *p_header, size_t i_header_length, size_t i_win_off, size_t i_win_length, int i_peer_quality);
    ssize_t Send_Piece(size_t i_piece_quality, size_t i_piece_id, block_t *p_piece);
    int GetOutBufferMaxSize();
    ssize_t Flush();
    size_t GetOutBufferCount();
    void Close();
    size_t RemoveBytes(size_t i_bytes_count);
    ssize_t Send_Bitfield(size_t i_win_offset, size_t i_length, char *p_bitfield);
    size_t GetInBufferCount();
    ssize_t Send_Request(size_t i_piece_quality, size_t i_piece_id);
    char * GetBufferedMessage(size_t *const i_length);
    unsigned int Send_Cancel(size_t i_piece_quality, size_t i_piece_id);
    ssize_t Send_State(unsigned __int8 state);
    int GetBufferedMessageLength();
    ssize_t Feed();
    ssize_t Feed(Rate *rate);
    bool HasBufferedMessage();
    int GetBufferedMessageType();
    SOCKET GetSocket();

    goalbit_t_0* p_this;
    int sock;
    int sock_was;
    int m_oldbytes;
    btBuffer buffer_in;
    btBuffer buffer_out;
};

struct metadata_piece_t {
    int i_piece_id;
    int i_duration;
    int64_t i_offset;
    int64_t i_size;
    char* psz_MD5;
};

class MetadataParser {
public:
    MetadataParser(goalbit_t_0 *pT_0);

    metadata_keyframe_t* parseKeyframe(char **p_begin, char *p_end);
    metadata_piece_t* parsePiece(char **p_begin, char *p_end, uint64_t *i_piece_offset);
    metadata_segment_t* parseSegment(char **p_begin, char *p_end);
    metadata_content_t* parseMetadata(char *p_buffer, size_t i_length, const char *psz_url);
    metadata_stream_t* parseStream(char **p_begin, char *p_end);

    goalbit_t_0* p_this;
};

class MetadataManager;

class MetadataContent {
public:
    MetadataContent(goalbit_t_0*);
    MetadataContent(goalbit_t_0 *p_goalbit, MetadataManager *p_manager);
    ~MetadataContent();
    array_t<metadata_piece_t>* getSegmentPieces(const int i_current_stream, const int i_segment);
    int getPieceSize(const int i_current_stream, const int i_segment, const int i_piece_id);
    unsigned int getLastSegment(const int i_current_stream);
    int getSegmentSize(const int i_current_stream, const int i_segment);
    void print();
    long double loadMetadata(const char *psz_metadata_url);
    metadata_content_t* readMetadata(const char *psz_metadata_url);
    long double updateMetadata(const char *psz_metadata_url);
    void Run();
    metadata_piece_t* getSegmentPiece(const int i_current_stream, const int i_segment, const int i_piece);
    char* getPieceMD5(const int i_current_stream, const int i_segment, const int i_piece_id);
    array_t<metadata_keyframe_t>* getSegmentKeyframes(const int i_current_stream, const int i_segment);
    unsigned int getPieceSegment(int i_piece_id);
    int segmentID2PieceID(const int i_segment_id, const int i_stream, const int i_criteria, const int i_order, int *i_selected_segment_id);
    char* getSegmentMD5(const int i_current_stream, const int i_segment);

    goalbit_t_0* p_this;
    MetadataManager* p_parent;
    MetadataParser* p_parser;
    metadata_content_t* p_content;
    int64_t t_next_metadata_update;
    hash_t_0* p_pieces;
};

typedef struct exec_info_t_0 {
    int i_quality;//hls stream index
    int i_segment_id;// init initial_sequence(hls max sequence - 5)
    int i_segment_offset;
    bool b_segment_completed;
    float_t f_segment_played;
    int64_t i_played_time;
    float_t f_abd;// init 0//
    int i_abi;// init initial_sequence(hls max sequence - 5)
} exec_info_t;

struct metadata_keyframe_t {
    int i_offset;
    int i_duration;
};

#define PEER_STATUS unsigned char

class BasicPeer {
public:
    void SetAddress(sockaddr_in addr);
    void SetPort(sockaddr_in addr);
    void SetIp(sockaddr_in addr);

    bool AddrEquiv(sockaddr_in addr);
    BasicPeer(goalbit_t_0 *p_goalbit);

    Rate* p_rate_dl;
    Rate* p_rate_ul;
    int m_latency;
    goalbit_t_0* p_this;
    sockaddr_in m_sin;
};

class Peer : public BasicPeer {
public:
    int ProcessInterested(size_t a1, char* a2);
    unsigned int ProcessWindowsUpdate(size_t i_msg_length, char *p_msg_data);
    unsigned int ProcessCancelPiece(size_t i_msg_length, char *p_msg_data);
    int ProcessPiece(size_t i_msg_length, char *p_msg_data);
    int ProcessRequest(size_t i_msg_length, char *p_msg_data);
    int ProcessBitfield(size_t i_msg_length, char *p_msg_data);
    int ProcessHave(size_t i_msg_length, char *p_msg_data);
    int IsEmpty(size_t i_quality);
    int NeedWrite(int limited);
    int NeedRead(int limited);
    int Need_Local_Data();
    int AreYouOK();
    void Prefetch(time_t deadline);
    bool NeedPrefetch();
    Peer(goalbit_t_0 *p_goalbit);
    int CancelPiece(size_t i_piece_id);
    int Send_ShakeInfo();
    int HealthCheck();
    bool CheckSendStatus();
    int ResponsePiece();
    int CouldReponsePiece();
    int SendPiece();
    void CloseConnection();
    int ReceiveHandShake();
    unsigned int SendRequest();
    unsigned int RequestPiece();
    int ProcessUnchoke();
    int ProcessChoke();
    unsigned int ProcessEndOfStream(size_t i_msg_length, char *p_msg_data);
    int CancelRequestedPieces();
    int SetLocal(int s);
    int Need_Remote_Data();
    int RequestCheck();
    int ReceivePiece();
    unsigned int ReceiveMessage();
    int ReportCompletePiece(size_t piece_quality, size_t i_piece_id);

    //BasicPeer baseclass_0;
    time_t m_last_timestamp;
    int m_unchoke_timestamp;
    unsigned char _peerF28;//init 0//0b10000000
        //2 bit:1->connect ok, 0->Operation now in progress
        //xxx0011x(3):close
        //xxx0010x(2):send request
        //xxx0001x(1):send Handshake
        //1110xxxx():
        //x0xxxxxx:? after close
    unsigned char _bf29;//ini 0
    int m_err_count;
    int m_req_send;
    int m_req_out;
    int m_prev_dlrate;
    int m_latency_timestamp;
    int m_health_time;
    int m_receive_time;
    int m_next_send_time;
    int m_lastmsg;
    int m_choketime;
    int m_prefetch_time;
    int m_cancel_time;
    int i_win_offset;
    int i_win_length;
    int score;
    PEER_STATUS m_state;//init 0b101
        // final 1st bit : choke    // fe : unchoke
        // final 2st bit : interest // fd : not interest, unchoke
        // final 3st bit : local
        // final 4st bit : interest
    char id[20];
    btBitField* bitfield;
    btStream* stream;
    PeerRequestQueue* p_request_q;
    PeerRequestQueue* p_reponse_q;
    int i_ab_index;
    int i_peer_quality;
};

class PeerList {
public:
    int UnChokeCheck(Peer *peer, Peer **peer_array);
    int FillFDSet(fd_set *rfdp, fd_set *wfdp, bool bkeepalive_check, bool bunchoke_check, Peer **UNCHOKER);
    void Accepter();
    void UpdatePeersWindow(size_t i_new_offset);
    size_t GetDownloads();
    void CancelPiece(size_t idx);
    void Tell_World_I_Have(size_t i_quality, size_t idx);
    void DeleteExpiredRequest(size_t idx);
    int AlreadyHaveThisPeer(unsigned __int8 *psz_peer_id);
    int AlreadyRequested(size_t i_quality, size_t piece_id);
    void CheckP2PBitFieldFromP2P(btBitField *bf);
    void CheckPeersRequest();
    void AnyPeerReady(fd_set *rfdp, fd_set *wfdp, int *nready, fd_set *rfdnextp, fd_set *wfdnextp);
    void AnyNewPeer();
    long double WaitBW();
    int BandWidthLimitDown(double when);
    int IsIdle();
    void SetUnchokeIntervals();
    size_t GetUnchoked();
    int BandWidthLimitUp(double when);
    unsigned int NewPeer(sockaddr_in addr, SOCKET sk, unsigned int score);
    bool NeedMorePeers();
    void CloseConnectionByPeerType(bool b_seeder, int i_num_to_close);
    int IntervalCheck(fd_set *rfdp, fd_set *wfdp);
    PeerList(goalbit_t_0 *p_goalbit);
    int Initial_ListenPort();
    void CloseAll();

    goalbit_t_0* p_this;
    generic_queue_t_0* p_newsocks_queue;// p2p incoming socket and addr
    bool b_accept_new_conn;// init false
    pthread_t* p_listener_thread;
    pthread_mutex_t listener_lock;
    pthread_t* p_upnp_thread;
    int m_listen_sock;
    BT_PEERNODE* m_head;
    BT_PEERNODE* m_dead;
    int m_peers_count;//init 0
    int m_seeds_count;//init 0
    int m_leechers_count;//init 0
    int m_conn_count;
    int m_max_unchoke;
    int i_leechers_unchoke;
    int i_seeders_unchoke;
    int i_opt_unchoke;
    int m_unchoke_check_timestamp;//init time()
    int m_keepalive_check_timestamp;//init time()
    int m_last_progress_timestamp;
    int m_opt_timestamp;//init time()
    int m_interval_timestamp;//init time()
    int m_unchoke_interval;
    int m_opt_interval;
    int m_check_slots_interval;//init time()
    int m_defer_count;
    int m_missed_count;
    int m_upload_count;
    int m_prev_limit_up;
    char m_listen[22];
    unsigned char _peerlistF9e;//init 0b1100111
        //bit final 1:BandWidthLimitUp(0)
        //bit final 2,4:BandWidthLimitDown(p_this->p_structure->Self->p_rate_dl->m_late)
        //bit final 3,5:BandWidthLimitUp(p_this->p_structure->Self->p_rate_dl->m_late)
    void UnchokeIfFree(Peer *pPeer);
};

struct BT_PEERNODE {
    Peer* peer;
    BT_PEERNODE* next;
};

struct metadata_stream_t {
    char* psz_name;
    int i_bitrate;
    int i_video_width;
    int i_video_height;
    array_t<metadata_segment_t>* p_segments;
};

struct metadata_segment_t {
    int i_sequence;
    int i_duration;
    bool b_discontinuity;
    int64_t i_size;
    char* psz_MD5;
    array_t<metadata_piece_t>* p_pieces;
    array_t<metadata_keyframe_t>* p_keyframes;
};

struct hls_segment_t {
    int i_sequence;
    float_t f_duration;
    int64_t i_size;
    int64_t i_bitrate;
    int64_t t_downloaded;
    int64_t t_download;
    bool b_discontinuity;
    char* psz_url;
    char* psz_desc;
    char* psz_date;
    block_t* p_data;
    pthread_mutex_t lock;
};

struct hls_content_t {
    char* psz_main_url;
    bool b_is_live;//init true
    bool b_has_metafile;
    int i_version;//init 1
    array_t<hls_stream_t>* p_streams;
};

struct metadata_content_t {
    char* psz_metadata_url;
    int i_program_id;
    array_t<metadata_stream_t>* p_streams;
};

typedef struct piece_ready_t_0 {
    int i_stream;
    int i_piece;
    int64_t t_downloaded;
    block_t* p_data;
} piece_ready_t;

typedef struct {
    size_t i_quality;
    size_t i_segment_id;
} quality_segmentid;

typedef struct {
    int i_stream;
    int i_piece;
} stream_piece;

typedef struct {
    int i_stream;
    int i_segment;
} stream_segment;

class MetadataManager {
public:
    MetadataManager(goalbit_t_0 *p_goalbit);
    ~MetadataManager();
    uint64_t getPieceSize(const int i_current_stream, const int i_segment, const int i_piece_id);
    int getBiggestPieceID(const int i_stream);
    uint64_t getSegmentSize(const int i_current_stream, const int i_segment);
    void setReady(bool ready);
    metadata_piece_t* getPiece(const int i_current_stream, const int i_segment, const int i_piece);
    char* getPieceMD5(const int i_current_stream, const int i_segment, const int i_piece_id);
    char* getSegmentMD5(const int i_current_stream, const int i_segment);
    array_t<metadata_keyframe_t>* getSegmentKeyframes(const int i_current_stream, const int i_segment);
    array_t<metadata_piece_t>* getSegmentPieces(const int i_current_stream, const int i_segment);
    int getPieceSegment(const int i_piece_id);
    int segmentID2PieceID(const int i_segment_id, const int i_stream, const int i_criteria, const int i_order);
    bool isReady();

    goalbit_t_0* p_this;
    pthread_mutex_t content_lock;
    pthread_t* p_metadata_thread;
    bool b_ready;
    MetadataContent* p_metadata_content;
};

struct hls_stream_t {
    int i_program_id;
    int i_version;
    int64_t i_media_sequence;
    float_t f_target_duration;
    long long i_bitrate;
    int64_t i_total_size;
    char* psz_codecs;
    char* psz_resolution;
    bool b_cache;
    bool b_end_list;
    char* psz_manifest_url;
    array_t<hls_segment_t>* p_segments;
    pthread_mutex_t lock;
};

class HLSParser {
public:
    HLSParser(goalbit_t_0 *p_goalbit);
    hls_stream_t *parseStreamInformation(char *p_read, const char *url, const char *uri);
    array_t<hls_stream_t> *parseMetaPlaylist(char *p_buffer, size_t i_length, int i_version, const char *psz_url);
    hls_content_t * parseManifest(char *p_buffer, size_t i_length, const char *psz_url);
    long double parseTargetDuration(char *p_read);
    int parseSegmentInformation(char *p_read, int version, float *duration, char **desc);
    int parseVersion(char *p_read);
    int parseDiscontinuity(char *p_read);
    int parseAllowCache(char *p_read);
    hls_stream_t* parsePlaylist(char *p_buffer, size_t i_length, int i_version, const char *psz_uri);
    char* parseProgramDateTime(char *p_read);
    int parseMediaSequence(char *p_read);
    goalbit_t_0 *p_this;
};

struct goalbit_quality_info_t_0 {
    int i_quality;
    int64_t i_bitrate;
    char* psz_resolution;
};

struct goalbit_qualities_desc_t_0 {
    int i_quality_num;
    goalbit_quality_info_t_0* p_qualities;
};

typedef struct segment_download_info_t_0 {
    char* psz_url;
    float f_duration;
    long long i_bitrate;
    bool b_is_downloaded;
} segment_download_info_t;

class HLSContent {
public:
    HLSContent(goalbit_t_0 *p_goalbit, HLSManager *p_manager);
    ~HLSContent();
    void setSegmentData(const int i_current_stream, const int i_chosen_sequence, hls_segment_t *p_new_segment);
    int downloadHLSPiece(piece_request_info_t_0 *p_request);
    hls_segment_t * readSegment(const char *psz_segment_url, const int i_max_size, const float f_duration);
    bool getSegmentDownloadInfo(const int i_current_stream, const int i_chosen_sequence, segment_download_info_t_0 *p_download_info);
    int downloadHLSSegment(const int i_current_stream, const int i_chosen_sequence);
    hls_stream_t *readStreamManifest(const char *psz_manifest_url);
    int updateHLSManifest();
    void ContentDownloaderControl(const int i_action, piece_request_info_t *p_data);
    void cleanSegmentData(const int i_current_stream, const int i_chosen_sequence);
    long double getSegmentDuration(const int i_current_stream, const int i_chosen_sequence);
    hls_segment_t *getSegmentData(const int i_current_stream, const int i_chosen_sequence);
    int getStreamCount();
    int getMaxSegment(const int i_current_stream);
    int getMinSegment(const int i_current_stream);
    goalbit_qualities_desc_t_0* exportStreamsDescription();
    void ContentDownloaderRun();

    bool isLiveContent();
    void ManifestUploaderRun(void);
    int loadHLSManifest(const char *psz_manifest_url);
    hls_content_t* readMainManifest(const char *psz_manifest_url);

    goalbit_t_0* p_this;
    HLSManager* p_parent;
    HLSParser* p_parser;
    hls_content_t* p_content;
    long long t_next_hls_update;
    generic_queue_t_0* p_events_queue;
};

typedef struct hash_entry_t_0 {
    char* psz_key;
    void * p_value;
    hash_entry_t_0* p_next;
} hash_entry_t;

typedef struct hash_t_0 {
    int i_size;
    hash_entry_t** p_entries;
} hash_t;

struct bandwidth {
    int64_t timestamp;
    int bytes;
};

#define KEY_SP '|'	//the keyname list's delimiters
#define KEYNAME_SIZ 32
#define KEYNAME_LISTSIZ 256

#define MAX_INT_SIZ 64

#define QUERY_STR 0
#define QUERY_INT 1
#define QUERY_POS 2
#define QUERY_LONG 3

#define PEER_ID_LEN 20

class BWRate {
public:
    void addBytes(size_t i_bytes, mtime_t t_time);
    BWRate();
    void reset();
    uint64_t getTotalBytes();

    int64_t i_total_bytes;
    int i_total_requests;
    bandwidth history[4];
    int i_history_last;
};

typedef struct piece_downloaded_info_t_0 {
    int64_t t_downloaded;
    void* p_data;
} piece_downloaded_info_t;

class HLSManager {
public:
    HLSManager(goalbit_t_0 *p_goalbit);
    ~HLSManager();
    void notifySegmentError(const int i_stream, const int i_segment);
    void setReady(bool ready);
    unsigned int downloadSegment(const int i_stream, const int i_segment);
    void notifyPieceReady(piece_request_info_t_0 *p_request, piece_downloaded_info_t_0 *p_downloaded_info);
    void notifySegmentReady(const int i_stream, const int i_segment, bool b_downloaded);
    unsigned int downloadPiece(const int i_stream, const int i_piece, const int i_segment, uint64_t i_offset, uint64_t i_size);
    void cleanSegment(const int i_stream, const int i_segment);
    array_t<quality_segmentid> * getPiecesReadyAndRequested();
    array_t<quality_segmentid> * getSegmentsReadyAndRequested();
    int isPieceRequestedOrReady(const int i_piece);
    int isSegmentRequestedOrReady(int i_segment);
    int getMaxSegmentID(const int i_stream);
    long double getSegmentDuration(const int i_stream, const int i_segment);
    piece_ready_t* getNextPieceReady();
    bool hasPiecesReady();
    hls_segment_t *getSegment(const int i_stream, const int i_segment);
    quality_segmentid *getNextSegmentReady();
    bool hasSegmentsReady();
    int getInitialSequence();
    goalbit_qualities_desc_t_0 * exportStreamsDescription();
    bool isLiveContent();
    bool isReady();

    goalbit_t_0* p_this;
    pthread_mutex_t content_lock;
    pthread_t* p_hls_manifest_thread;
    pthread_t* p_hls_segment_thread;
    bool b_ready;//init 0
    pthread_mutex_t segments_lock;
    hash_t_0* p_segment_requests;
    array_t<quality_segmentid>* p_segments_ready;
    hash_t_0* p_piece_requests;
    array_t<piece_ready_t>* p_pieces_ready;
    HLSContent* p_hls_content;
};

struct TrackerServer {
    void sendHTTPrequestToTracker(goalbit_t *p_this, char *psz_request_url);
    void Run();
    TrackerServer(goalbit_t *p_goalbit, btTracker *p_tracker);

    goalbit_t* p_this;
    btTracker* p_parent;
    generic_queue_t_0* p_events_queue;
};

struct _bwsample {
    int64_t timestamp;
    int bytes;
};

class Rate {
public:
    void CountAdd(size_t nbytes);
    int CurrentRate();
    void StopTimer();
    void StartTimer();
    void RateAdd(size_t nbytes, size_t bwlimit, double timestamp);
    void Cleanup();
    Rate(goalbit_t_0 *p_goalbit);
    size_t RateMeasure();
    void Reset();

    goalbit_t_0* p_this;
    int m_last_timestamp;
    int m_total_timeused;
    int64_t m_count_bytes;
    time_t m_last_realtime;
    time_t m_recent_realtime;
    time_t m_prev_realtime;
    int64_t m_last_size;
    int64_t m_recent_size;
    int64_t m_prev_size;
    int64_t m_late;//init 0.0
    unsigned char _bf40;//init 0b11111110
    _bwsample p_history[30];
    int i_history_last;
    Rate* m_selfrate;

};

typedef struct generic_queue_t_0 {
    pthread_mutex_t lock;
    queue_node_t_0* p_last;
    queue_node_t_0* p_first;
} generic_queue_t;

typedef struct generic_event_t_0 {
    int i_event;
    value_t value;
} generic_event_t;

typedef struct queue_node_t_0 {
    generic_event_t event;
    queue_node_t_0* p_next;
} queue_node_t;

class btTracker {
public:
    int Initial();
    int BuildBaseRequest();
    void setTrackerResponse(block_t *p_response);
    btTracker(goalbit_t_0 *p_goalbit);
    void SetStoped();
    int SendRequest();
    void Reset(time_t new_interval);
    unsigned int UpdatePeerList(uint8_t *buf, size_t bufsize);
    void IntervalCheck();

    goalbit_t_0* p_this;
    char* pzs_base_url;
    pthread_mutex_t trackercom_lock;
    pthread_t* p_tracker_server;
    block_t* p_tracker_responce;//init 0
    char m_key[9];
    char m_trackerid[21];
    unsigned char _trackerF46;
        //0b11000000:init
        //xxxxxx01:send request(queue)
        //xxxxx1xxx:stop
        //xxxxxx11:
    int m_interval;//init 15
    int m_default_interval;
    time_t m_last_timestamp;
    int m_last_read;
    bool b_startup_buff_reported;
    bool b_check_port;//init false
    int m_ok_click;
    int m_prevpeers;
    int i_broadcasters_count;
    int i_superpeers_count;
    int i_peers_count;
    bool b_buff_event;
    TrackerServer* p_server;
};

struct IPLIST {
    sockaddr_in address;
    int score;
    IPLIST* next;
};

class IpList {
public:
    unsigned int Pop(sockaddr_in *psin, unsigned int *i_peer_score);
    unsigned int Add(sockaddr_in *psin, unsigned int i_peer_score);
    void _Emtpy();

    void* p_this;
    IPLIST* ipl_head;
    int count;
};

//struct UPNPUrls {
//    char* controlURL;
//    char* ipcondescURL;
//    char* controlURL_CIF;
//    char* controlURL_6FC;
//    char* rootdescURL;
//};

//struct IGDdatas_service {
//    char controlurl[128];
//    char eventsuburl[128];
//    char scpdurl[128];
//    char servicetype[128];
//};
//
//struct IGDdatas {
//    char cureltname[128];
//    char urlbase[128];
//    char presentationurl[128];
//    int level;
//    IGDdatas_service CIF;
//    IGDdatas_service first;
//    IGDdatas_service second;
//    IGDdatas_service IPv6FC;
//    IGDdatas_service tmp;
//};

struct goalbit_structure_t {
    bool b_btc_alive;//init 1
    bool b_check_win;// init false
    time_t now;
    pthread_mutex_t exec_lock;
    bool b_buffering;// init true // set false if duration for downloaded data exceed buffering time
    bool b_p2p_syncing;// init true // synchronizing meta data and hls data
    long long i_p2p_sync_start;
    exec_info_t exec_info;
    bool b_fixed_quality;
    int i_target_quality;//hls stream index
    mtime_t i_last_buff_start;// init time, o when StartExecution
    int64_t i_startup_buff_dur;// init 0, duration to first call StartExecution
    int i_buff_num;// init 0, inc when StartExecution
    int64_t i_total_rebuff_dur;// init 0
    int64_t f_qoe;
    bool b_hls_live;
    size_t i_hls_qualities;//stream count?
    int i_hls_win_offset;// init 0x80000001//starting segment id
    int i_hls_win_length;//init param->i_window_length 18
    float_t f_hls_buffer_size;//default 8
    float_t f_hls_urgent_size;//default 12
    bool b_hls_can_down_pieces;//default true
    HLSManager* p_hlsManager;
    MetadataManager* p_metadataManager;
    BWRate* p_hlsRate;
    int64_t i_last_sync_date;
    int i_last_quality_change;
    int i_quality_meter;
    bool b_p2p_enabled;// init false
    char* psz_p2p_metadata_url;
    int i_p2p_win_offset;
    int i_p2p_win_length;
    int i_eos_piece_id;
    bool b_eos_event_sent;
    char psz_internal_ip[0x10];// get local ip
    UPNPUrls* p_urls;
    IGDdatas* p_data;
    int i_external_port;
    int i_check_port_opened;//init 1
    int i_tracker_view_port_opened;//init 1
    int p_trackercom_manager;
    int64_t i_last_exec_report;
    int64_t i_last_bitfield_update;
    int cfg_req_queue_length;
    int cfg_max_listen_port;// init 3706
    int cfg_min_listen_port;// init 2706
    char* cfg_user_agent;//"LIBGBSP/1.0.0"
    char* arg_user_agent;//"-CD0302-"
    Peer* g_next_up;
    Peer* g_next_dn;
    BasicPeer* Self;
    btContent* BTCONTENT;
    IpList* IPQUEUE;
    btTracker* Tracker;
    PeerList* WORLD;
    PeerPendingQueue* PENDINGQUEUE;
    float_t f_hls_bitrates[10];
    float_t f_hls_lengths[10];
    float_t f_hls_durations[10];
    float_t f_p2p_bitrates[10];
    float_t f_p2p_lengths[10];
    float_t f_p2p_durations[10];
    int i_history_count;
    float_t f_avg_hls_bitrate;
    float_t f_avg_hls_length;
    float_t f_avg_usec_x_segment;
    float_t f_avg_segment_x_sec;
    float_t f_avg_p2p_bitrate;//init 0
    float_t f_avg_p2p_length;
    float_t f_avg_usec_x_piece;
    float_t f_avg_piece_x_sec;
    char* psz_log_file;
    FILE* p_log_file;
};

class PeerPendingQueue {
public:
    int DeletePieceByID(size_t i_piece_id);
    int AddPiece(size_t i_piece_quality, size_t i_piece_id);
    int DeletePieceByIndex(int i_index);
    int GetPieces(PeerRequestQueue *p_requests, btBitField *p_bitfield);
    int getFirstEmptyIndex();
    int ExistPiece(size_t i_piece_quality, size_t i_piece_id);
    PeerPendingQueue(goalbit_t_0 *p_goalbit);
    int AddPieces(PeerRequestQueue *p_requests);

    goalbit_t_0* p_this;
    PIECE* pending_array[100];
    int i_count;
};

class PIECE {
public:
    int i_index;
    int i_quality;
    int i_length;
    int t_request_time;
    PIECE* p_next;
};

class PeerRequestQueue {
public:
    PeerRequestQueue(goalbit_t_0 *p_goalbit);

    time_t GetRequestTime(size_t i_piece_quality, size_t i_piece_id);
    size_t GetSmallestQuality();
    size_t GetSmallestIndex();
    int HasPiece(size_t i_piece_quality, size_t i_piece_id);
    void SetSend(PIECE *p_piece);
    void SetRequestTime(PIECE *p_piece, time_t t_time);
    PIECE *GetSend();
    int Add(size_t i_piece_quality, size_t i_piece_id, size_t i_piece_length, time_t t_time);
    size_t GetRequestLength(size_t i_piece_quality, size_t i_piece_id);
    int Remove(size_t i_piece_quality, size_t i_piece_id);
    size_t GetHeadQuality();
    size_t GetHeadIndex();
    PIECE* GetHead();
    void Clear();
    bool IsEmpty();
    size_t Size();

    void* p_this;
    PIECE* p_head;
    PIECE* p_send;

    bool IsValidRequest(int i);
};

typedef block_t goalbit_block_t;
//struct goalbit_block_t {
//    int i_buffer_size;
//    int i_buffer_index;
//    void* p_buffer;
//};

union goalbit_value_t {
    int i_int;//4
    bool b_bool;
    float_t f_float;
    char* psz_string;
    char* p_address;
    void* pos;
    goalbit_qualities_desc_t_0* goalbit_qualities_desc_t_01;//3
};

typedef struct goalbit_event_t {
    int i_event;
    GoalBitSession* p_ref;
    goalbit_value_t g_value;
} goalbit_event_t_0;

struct SHA1_CTX_0 {
    uint32_t state[5];
    int count[2];
    unsigned __int8 buffer[100];
};

typedef struct goalbit_param_t_0 {
    bool b_verbose;//default 1
    bool b_log_to_file;//default 1
    int i_verbose_level;//default 3
    char* psz_p2p_tracker_url;
    char* psz_hls_server_url;
    char* psz_meta_server_url;
    char* psz_content_id;
    char* psz_base_dir;
    char* psz_log_dir;//init 0
    bool b_listen_for_connections;
    bool b_disable_upnp;
    int i_closure_type;
    int i_cache_size;//init 0x1000000
    int i_buffer_size;
    int i_urgent_size;
    int i_window_length;
    float_t f_exponential_beta;
    int i_max_peers_from_tracker;//init 100
    int i_max_leecher_list;//init 30
    int i_max_seed_list;//init 0
    int i_min_peers_list;//init 1
    int i_max_leecher_unchoke;//init 4
    int i_max_seeder_unchoke;//init 0
    int l_listen_ip;//init 0
    int i_listen_port;// init 0
    int i_max_bandwidth_down;//init 0
    int i_max_bandwidth_up;//init 0
    //void* (*pf_event_callback)(goalbit_t *, goalbit_event_t *);//goalbit_event_callback
    GoalBitSession* p_external_ref;
} goalbit_param_t;

class GoalBitSession {
public:
    GoalBitSession(int _session_id, session_t _session_type, char *_content_id, char *_p2p_tracker_url, char *_hls_server_url, char *_p2p_manifest_url, int _start_buffer_size, int _urgent_buffer_size);
    ~GoalBitSession();
    int GetQuality();
    void SetQuality(int quality_id);
    int GetBufferingPercentage();
    goalbit_qualities_desc_t_0 *GetQualitiesDesc();
    int GetSessionId();
    bool IsReady();
    bool IsOk();
    void GetPieceContent(int piece_id, uint8_t **piece_content, int *size);
    goalbit_block_t * Prebuffer(int *error);
    goalbit_block_t * GetNextPiece(int *error);
    session_t GetSessionType();
    void GetPieces(int **pieces_ids, float **pieces_durations, bool **pieces_disconts, int *size, float *max_duration);
    time_t GetLastUsedTime();
    bool CanPrebuffer();
    void ReadPieces();
    void _AddGBTPv2DSegment(goalbit_block_t *p_chunk, goalbit_segment_info_t_0 segment_info);
    void SetBufferingState(bool b_buff);
    void SetBufferingPercentage(int percentage);
    void SetQualitiesDesc(goalbit_qualities_desc_t_0 *p_qualities);

private:
    bool b_gbtpv2d_buffering;// init true// buffering status
public:
    int session_id;
    session_t session_type;
    char* content_id;
    char* p2p_tracker_url;
    char* hls_server_url;
    char* p2p_manifest_url;
    int start_buffer_size;
    int urgent_buffer_size;
    goalbit_param_t* p_param;
    goalbit_t* p_goalbit;
    GoalBitPiece* pGoalBitPiece[12];
    int piece_count;//pGoalBitPiece count//init 0
    int last_used_time;
    int next_read_time;// init 0
    int i_gbtpv2d_buff_perc;
    int i_gbtpv2d_last_quality;
    //bool b_gpa_buffering;// init true
    bool b_gpa_prebuffered;
    int i_gpa_exec_index;// a number of calling GoalBitSession::ReadPieces
    goalbit_qualities_desc_t_0* p_qualities_desc;
};

class GoalBitManager {
public:
    int GetSessionQuality(int session_id);
    void SetSessionQuality(int session_id, int quality_id);
    int GetBufferingPercentage(int session_id);
    goalbit_qualities_desc_t_0 *GetQualitiesDesc(int session_id);
    GoalBitManager();
    ~GoalBitManager();
    session_t GetSessionType(int session_id);
    bool IsReady(int session_id);
    void GetPieces(int session_id, int **pieces_ids, float **pieces_durations, bool **pieces_disconts, int *size, float *max_duration);
    void GetPieceContent(int session_id, int piece_id, uint8_t **piece_content, int *size);
    int GetSessionIndex(int session_id);
    bool CanPrebuffer(int session_id);
    goalbit_block_t* Prebuffer(int session_id, int *err);
    goalbit_block_t* GetNextPiece(int session_id, int *err);
    int NewSession(session_t session_type, char *content_id, char *p2p_tracker_url, char *hls_server_url, char *p2p_manifest_url, int start_buffer, int urgent_buffer);

    int global_sessions_id;
    GoalBitSession* goalbit_sessions[5];
    int sessions_count;
    pthread_t* control_thread;
    pthread_mutex_t mutex;
    bool running;// goalbit running

};

extern void* (*Curl_cmalloc)(size_t);
extern void (*Curl_cfree)(void*);
extern void* (*Curl_crealloc)(void*, size_t);
extern char* (*Curl_cstrdup)(const char*);
extern void* (*Curl_ccalloc)(size_t, size_t);

#endif //STREA