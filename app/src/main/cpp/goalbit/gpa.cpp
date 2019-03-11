
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/stat.h>
#include <miniupnpc.h>
#include "gpa.h"
#include "mongoose.h"
#include "func.h"
#include "upnpcommands.h"
#include "./../sha1.c"

char goalbitp2p[] = "GOALBITP2P";
extern void print_msg(const char* a1, const char *a2, ...);
extern void print_err(const char* a1, const char *a2, ...);
extern void print_dbg(const char* a1, const char *a2, ...);
extern void print_wrn(const char* a1, const char *a2, ...);

unsigned char BIT_HEX[] = {
        0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1
};

int initialized = 0;
int init_flags;
int randseed;
bool ares_initialized;
int ares_init_flags;
GoalBitManager* goalBitManager;//dword_70A0;
//char* dword_70A4;
char* libgbsp_base_dir;//70A8
char* libgbsp_log_dir;//70AC
int androidVersion;
pthread_t dword_70B4;
pthread_mutex_t unk_70B8;
bool bStopGoalbit;
char* dword_70C0;
//char* dword_70CC[0x12];
//JavaVM* jvm;
char* dword_7144;
//mg_context* dword_7148;
char byte_8171E59[7];
unsigned char byte_81508E2;

void Mypthread_mutex_lock(int n, pthread_mutex_t* t)
{
    if ( n == -1 ) {
        pthread_mutex_lock(t);
        return;
    }
    //    if (n == 49 || n == 40 || n == 37 || n == 50 || n == 36 || n == 39 || n == 41 || n == 51 || n == 48 || n == 45 || n == 46 || n == 44 || n == 38 || n == 43)
    //        print_dbg(__func__, "%d %s\n", n, "mutex lock1");
    //print_dbg(__func__, "%d %s\n", n, "mutex lock1");
    pthread_mutex_lock(t);
    //print_dbg(__func__, "%d %s\n", n, "mutex lock2");
}

void Mypthread_mutex_unlock(int n, pthread_mutex_t* t)
{
    if ( n == -1 ) {
        pthread_mutex_unlock(t);
        return;
    }
    //print_dbg(__func__, "%d %s\n", n, "mutex unlock1");
    pthread_mutex_unlock(t);
    //print_dbg(__func__, "%d %s\n", n, "mutex unlock2");
}

/*
 void* msbtolsb(void* vpos1, void* vpos2, int cnt) {
 if (cnt < 2) {
 return 0;
 }
 char* pos1 = (char*)vpos1;
 char* pos2 = (char*)vpos2;
 int n = cnt-1;
 for (int i = 0; i < n; ++i) {
 char t = pos2[i];
 pos1[i] = pos2[n-i];
 pos1[n-i] = t;
 if (i == (n-i) || (i+1) == (n-i)) {
 break;
 }
 }
 return vpos1;
 }
 */

void* control_run(void *p_data)
{
    //int v1; // ebx@4
    //void *v2; // ebx@5
    int i; // [sp+14h] [bp-14h]@2
    //int j; // [sp+18h] [bp-10h]@7

    GoalBitManager * the = (GoalBitManager*)p_data;
    while (the->running)
    {
        Mypthread_mutex_lock(-1, &the->mutex);
        i = 0;
        while ( the->sessions_count > i )
        {
            if ( the->goalbit_sessions[i] )
            {
                /*
                 v1 = the->goalbit_sessions[i]->GetLastUsedTime() + 20;

                 #ifndef bDisabletimecheckforDebug
                 if (v1 < time(0)) {
                 //v2 = (void *)*((_DWORD *)p_data + i + 1);
                 if (the->goalbit_sessions[i + 1]) {
                 delete the->goalbit_sessions[i + 1];
                 //GoalBitSession::~GoalBitSession(*((GoalBitSession *const *)p_data + i + 1));
                 //operator delete(v2);
                 }
                 for (j = i + 1; the->sessions_count > j; ++j) {
                 the->goalbit_sessions[j] = the->goalbit_sessions[j +
                 1];//*((_DWORD *)p_data + j - 1 + 1) = *((_DWORD *)p_data + j + 1);
                 }
                 //*((_DWORD *)p_data + (*((_DWORD *)p_data + 6))-- - 1 + 1) = 0;
                 the->goalbit_sessions[j + 1] = 0;
                 the->sessions_count--;
                 continue;
                 }
                 #endif
                 */

                if ( the->goalbit_sessions[i]->GetSessionType() == HTTP_LIVE_STREAMING ) {
                    the->goalbit_sessions[i]->ReadPieces();
                }
            }
            ++i;
        }
        Mypthread_mutex_unlock(-1, &the->mutex);
        msleep(50000);
    }
    pthread_exit(0);
}

GoalBitManager::GoalBitManager()
{
    int i; // [sp+18h] [bp-10h]@1

    print_msg(__func__, "Start GoalBit Manager\n");
    this->global_sessions_id = 1;
    this->sessions_count = 0;
    this->running = 1;
    for ( i = 0; i <= 4; ++i ) {
        this->goalbit_sessions[i] = 0;
    }
    pthread_mutex_init(&this->mutex, 0);
    this->control_thread = new pthread_t();
    if ( pthread_create(this->control_thread, 0, control_run, (void*)this) )
    {
        print_msg(__func__, "Could not start control thread\n");
        this->control_thread = 0;
    }
}

int sub_39B8(mg_connection* v3, char* v4, int a3)
{
    //int v3; // r7@1
    //int v4; // r6@1
    struct tm *v6; // r0@1
    int result; // r0@1
    time_t timer; // [sp+10h] [bp-A0h]@1
    char s[0x80]; // [sp+14h] [bp-9Ch]@1
    //int v10; // [sp+94h] [bp-1Ch]@1

    //v3 = a1;
    //v4 = a2;
    //v5 = a3;
    //v10 = _stack_chk_guard;
    timer = time(0);
    v6 = gmtime(&timer);
    strftime(s, 0x80u, "%a, %d %b %Y %H:%M:%S GMT", v6);
    result = mg_printf(
            v3,
            "%s 200 OK\r\n"
                    "Date: %s\r\n"
                    "Server: %s\r\n"
                    "Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0\r\n"
                    "Pragma: no-cache\r\n"
                    "Content-Length: %d\r\n"
                    "Connection: close\r\n"
                    "Content-Type: %s\r\n"
                    "\r\n",
            "HTTP/1.1",
            s,
            "webserver/1.0",
            a3,
            v4);
    //if ( v10 != _stack_chk_guard )
    //    _stack_chk_fail(result);
    return result;
}

char* sub_3A70(mg_connection* a1, int a2)
{
    int v2; // r7@1
    //int v3; // r0@2
    //double v4; // r0@2
    //double v5; // r0@2
    int v6; // r0@2
    char *v7; // r4@2
    int v8; // r4@2
    signed int v9; // r3@2
    char *v10; // r5@4
    int v11; // r7@7
    float_t v12; // r0@7
    //int v13; // r1@7
    char *v14; // r5@7
    char *v15; // r5@7
    size_t v16; // r4@9
    //char* result; // r0@10
    mg_connection* v18; // [sp+10h] [bp-13C8h]@1
    int v19; // [sp+18h] [bp-13C0h]@1
    float v20; // [sp+1Ch] [bp-13BCh]@1
    int *ptr; // [sp+20h] [bp-13B8h]@1
    float *v22; // [sp+24h] [bp-13B4h]@1
    bool *v23; // [sp+28h] [bp-13B0h]@1
    char *src; // [sp+2Ch] [bp-13ACh]@2
    char *v25; // [sp+30h] [bp-13A8h]@9
    char s[5000]; // [sp+34h] [bp-13A4h]@1
    //int v27; // [sp+13BCh] [bp-1Ch]@1

    v2 = a2;
    v18 = a1;
    v20 = 0.0;
    v19 = 0;
    ptr = 0;
    v22 = 0;
    v23 = 0;
    memset(s, 0, 5000);
    goalBitManager->GetPieces(v2, &ptr, &v22, &v23, &v19, &v20);
    if ( v19 > 0 )
    {
        s[0] = 0;
        strcat(s, "#EXTM3U\r\n");
        strcat(s, "#EXT-X-VERSION:3\r\n");
        strcat(s, "#EXT-X-ALLOW-CACHE:NO\r\n");
        //v3 = _aeabi_f2d(LODWORD(v20));
        //LODWORD(v4) = _aeabi_dadd(v3);
        //v5 = floor(v4);
        v6 = ceil(v20);//v6 = _aeabi_d2iz(LODWORD(v5), HIDWORD(v5));
        asprintf(&src, "#EXT-X-TARGETDURATION:%d\r\n", v6);
        v7 = src;
        strcat(s, src);
        free(v7);
        v8 = 0;
        v9 = 1;
        while ( v8 < v19 )
        {
            if ( v9 )
            {
                asprintf(&src, "#EXT-X-MEDIA-SEQUENCE:%d\r\n", ptr[v8]);
                v10 = src;
                strcat(s, src);
                free(v10);
            }
            if ( v23[v8] )
                strcat(s, "#EXT-X-DISCONTINUITY\r\n");
            v11 = v8;
            v12 = v22[v8];//v12 = _aeabi_f2d(LODWORD(v22[v8]));
            asprintf(&src, "#EXTINF:%f, no desc\r\n", v12);
            v14 = src;
            ++v8;
            strcat(s, src);
            free(v14);
            asprintf(&src, "segment/%d.ts\r\n", ptr[v11]);
            v15 = src;
            strcat(s, src);
            free(v15);
            v9 = 0;
        }
        v16 = strlen(s);
        asprintf(&v25, "application/x-mpegURL");
        sub_39B8(v18, v25, v16);
        mg_printf(v18, "%s", s);
        print_dbg(__func__, s);
        free(v25);
        free(ptr);
        free(v22);
        free(v23);
    } else {
#ifdef GOALBIT_ANDROID
        mg_http_send_error(a1, 404, NULL);
#endif
    }
    return dword_70C0;
}

int GoalBitPiece::GetId() {
    return this->piece_id;
}

int GoalBitPiece::GetSize()
{
    return this->piece_size;
}

void GoalBitPiece::GetContent(uint8_t **content) {
    uint8_t *content_aux; // ST1C_4@1

    content_aux = (uint8_t *)malloc(this->piece_size);
    memcpy(content_aux, this->piece_content, this->piece_size);
    *content = content_aux;
}

double GoalBitPiece::GetDuration() {
    return this->piece_duration;
}

bool GoalBitPiece::GetDiscont() {
    return this->piece_discont;
}

GoalBitPiece::GoalBitPiece(int _piece_id, uint8_t *_piece_content, int _piece_size,
                           float _piece_duration, bool _piece_discont) {
    this->piece_id = _piece_id;
    this->piece_duration = _piece_duration;
    this->piece_size = _piece_size;
    this->piece_discont = _piece_discont;
    this->piece_content = (uint8_t *)malloc(this->piece_size);
    memcpy(this->piece_content, _piece_content, this->piece_size);

}

int GoalBitSession::GetSessionId()
{
    return session_id;
}

bool GoalBitSession::IsOk()
{
    return this->p_goalbit != 0;
}

void GoalBitSession::GetPieceContent(int piece_id, uint8_t **piece_content, int *size)
{
    bool v4; // al@4

    *size = 0;
    for ( int i = 0; i < this->piece_count; ++i )
    {
        v4 = this->pGoalBitPiece[i] && this->pGoalBitPiece[i]->GetId() == piece_id;
        if ( v4 )
        {
            *size = this->pGoalBitPiece[i]->GetSize();
            this->pGoalBitPiece[i]->GetContent(piece_content);
            break;
        }
    }
    this->last_used_time = time(0);
}

bool GoalBitSession::IsReady() {
    bool result; // eax@2
    session_t v2; // eax@3

    this->last_used_time = time(0);
    if ( !IsOk() )
    {
        result = false;
    }
    else
    {
        v2 = this->session_type;
        if ( v2 == HTTP_PROGRESSIVE_DOWNLOAD )
        {
            result = !this->b_gbtpv2d_buffering;
        }
        else if ( v2 == HTTP_LIVE_STREAMING )
        {
            result = this->b_gbtpv2d_buffering == false && this->piece_count;
        }
        else
        {
            result = false;
        }
    }
    return result;
}

bool GoalBitSession::CanPrebuffer() {
    return b_gpa_prebuffered;
}

block_t* goalbit_client_prebuffer(goalbit_t_0 *p_this, int *pi_status, goalbit_segment_info_t_0 *p_info)
{
    return p_this->p_structure->BTCONTENT->VideoPlayerPreBuffer(pi_status, p_info);
}

goalbit_block_t *GoalBitSession::Prebuffer(int *error) {
    goalbit_block_t *result; // eax@2
    goalbit_segment_info_t_0 segment_info; // [sp+1Ch] [bp-1Ch]@5
    int i_status; // [sp+28h] [bp-10h]@5
    goalbit_block_t *p_chunk; // [sp+2Ch] [bp-Ch]@1

    print_msg(__func__, " GoalBitSession::Prebuffer ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    p_chunk = 0;
    *error = 0;
    if ( this->p_goalbit )
    {
        if ( this->b_gbtpv2d_buffering )
        {
            *error = -2;
            result = p_chunk;
        }
        else
        {
            i_status = 0;
            p_chunk = goalbit_client_prebuffer(this->p_goalbit, &i_status, &segment_info);
            if ( i_status == 1 && p_chunk )
            {
                this->b_gpa_prebuffered = true;
                print_msg(__func__, " GoalBitSession::Prebuffer +++++++++++++++++++++++ piece buffered!");
                result = p_chunk;
            }
            else
            {
                *error = i_status;
                result = 0;
            }
        }
    }
    else
    {
        *error = -1;
        result = p_chunk;
    }
    return result;
}

block_t * goalbit_client_read(goalbit_t_0 *p_this, int *pi_status, goalbit_segment_info_t_0 *p_info)
{
    return p_this->p_structure->BTCONTENT->VideoPlayerRead(pi_status, p_info);
}

goalbit_block_t *GoalBitSession::GetNextPiece(int *error) {
    goalbit_block_t *result; // eax@2
    goalbit_segment_info_t_0 segment_info; // [sp+1Ch] [bp-1Ch]@5
    int i_status; // [sp+28h] [bp-10h]@5
    goalbit_block_t *p_chunk; // [sp+2Ch] [bp-Ch]@1

    p_chunk = 0;
    *error = 0;
    this->last_used_time = time(0);
    if ( this->p_goalbit )
    {
        if ( this->b_gbtpv2d_buffering )
        {
            *error = -2;
            result = 0;
        }
        else
        {
            i_status = 0;
            p_chunk = goalbit_client_read(this->p_goalbit, &i_status, &segment_info);
            if ( i_status == 1 && p_chunk )
            {
                print_msg(__func__, "GoalBitSession::GetNextPiece returning %.4f seconds of content.", segment_info.f_duration);
                result = p_chunk;
            }
            else if ( i_status == 3 )
            {
                result = 0;
            }
            else
            {
                *error = i_status;
                result = 0;
            }
        }
    }
    else
    {
        *error = -1;
        result = 0;
    }
    return result;
}

void GoalBitSession::GetPieces(int **pieces_ids, float **pieces_durations, bool **pieces_disconts,
                               int *size, float *max_duration) {
    int returned_size; // [sp+24h] [bp-24h]@1
    int *pieces_ids_aux; // [sp+30h] [bp-18h]@3
    float *pieces_durations_aux; // [sp+34h] [bp-14h]@3
    bool *pieces_disconts_aux; // [sp+38h] [bp-10h]@3

    *size = 0;
    *max_duration = 0;
    returned_size = 4;
    if ( this->piece_count <= 3 )
        returned_size = this->piece_count;
    pieces_ids_aux = (int *)malloc(sizeof(int*) * returned_size);
    pieces_durations_aux = (float *)malloc(sizeof(float*) * returned_size);
    pieces_disconts_aux = (bool *)malloc(sizeof(bool *) * returned_size);
    int j = 0;
    for (int i = this->piece_count - returned_size; this->piece_count > i; ++i )
    {
        pieces_ids_aux[j] = pGoalBitPiece[i]->GetId();//GoalBitPiece::GetId(*((GoalBitPiece *const *)&this->content_id + i + 8));
        pieces_durations_aux[j] = pGoalBitPiece[i]->GetDuration();//GoalBitPiece::GetDuration(*((GoalBitPiece *const *)&this->content_id + i + 8));
        pieces_disconts_aux[j] = pGoalBitPiece[i]->GetDiscont();//GoalBitPiece::GetDiscont(*((GoalBitPiece *const *)&this->content_id + i + 8));
        if ( pGoalBitPiece[i]->GetDuration() > *max_duration )
        {
            *max_duration = pGoalBitPiece[i]->GetDuration();
        }
        ++j;
    }
    *size = returned_size;
    *pieces_ids = pieces_ids_aux;
    *pieces_durations = pieces_durations_aux;
    *pieces_disconts = pieces_disconts_aux;
    this->last_used_time = time(0);
    return;
}

time_t GoalBitSession::GetLastUsedTime() {
    return this->last_used_time;
}

long long mdate()
{
    timeval tv_date; // [sp+28h] [bp-14h]@1

    gettimeofday(&tv_date, 0);
    long long result = tv_date.tv_sec;
    result = result*1000000;
    result += tv_date.tv_usec;
    return result;
}

void block_Release(block_t *pp_block)
{
    block_t *v1; // eax@1

    v1 = pp_block;
    if ( pp_block )
    {
        if ( v1->i_buffer_size )
        {
            if ( v1->p_buffer )
            {
                free(v1->p_buffer);
                v1 = pp_block;
            }
        }
        free(v1);
    }
}

void goalbit_block_release(goalbit_block_t *p_block)
{
    block_Release(p_block);
}

void GoalBitSession::ReadPieces() {
    goalbit_segment_info_t_0 segment_info; // [sp+3Ch] [bp-1Ch]@7
    goalbit_block_t *p_chunk; // [sp+48h] [bp-10h]@6
    int i_status; // [sp+4Ch] [bp-Ch]@6

    if ( this->p_goalbit && this->b_gbtpv2d_buffering == false)
    {
        if ( mdate() < this->next_read_time ) {
            return;
        }
        i_status = 0;
        if ( this->piece_count == 0 ) {
            p_chunk = goalbit_client_prebuffer(this->p_goalbit, &i_status, &segment_info);
            if ( i_status == 1  && p_chunk)
            {
                ++this->i_gpa_exec_index;
                _AddGBTPv2DSegment(p_chunk, segment_info);
                goalbit_block_release(p_chunk);
            }
        }
        p_chunk = goalbit_client_read(this->p_goalbit, &i_status, &segment_info);
        if ( i_status == 1 && p_chunk )
        {
            ++this->i_gpa_exec_index;
            if ( this->next_read_time )
                this->next_read_time += segment_info.f_duration * 1000000.0;
            else
                this->next_read_time = segment_info.f_duration * 1000000.0 + mdate();
            _AddGBTPv2DSegment(p_chunk, segment_info);
            goalbit_block_release(p_chunk);
        }
    }
}

uint8_t* block_Content(block_t *p_block)
{
    if ( p_block )
        return p_block->p_buffer;
    return 0;
}

uint8_t * goalbit_block_buffer(block_t *p_block)
{
    return block_Content(p_block);
}

size_t block_Total_Size(block_t *p_block)
{
    if ( p_block )
        return p_block->i_buffer_size;
    return 0;
}

size_t goalbit_block_size(block_t *p_block)
{
    return block_Total_Size(p_block);
}

void GoalBitSession::_AddGBTPv2DSegment(goalbit_block_t *p_chunk,
                                        goalbit_segment_info_t_0 segment_info) {
    //GoalBitPiece *v3; // ebx@7
    int i; // [sp+2Ch] [bp-1Ch]@11
    uint8_t *p_buffer; // [sp+30h] [bp-18h]@1
    size_t i_buffer_size; // [sp+34h] [bp-14h]@1
    GoalBitPiece *goalbit_piece; // [sp+38h] [bp-10h]@7
    bool b_discontinuity; // [sp+3Fh] [bp-9h]@2

    p_buffer = goalbit_block_buffer(p_chunk);
    i_buffer_size = goalbit_block_size(p_chunk);
    if ( segment_info.b_discontinuity )
        b_discontinuity = true;
    else
        b_discontinuity = this->piece_count != 0 && this->i_gbtpv2d_last_quality != segment_info.i_quality;
    print_msg(__func__, "[GPA] Adding %.2f seconds of content with ID: %u \n", segment_info.f_duration, this->i_gpa_exec_index);
    //v3 = (GoalBitPiece *)operator new(0x14u);
    goalbit_piece = new GoalBitPiece(
            this->i_gpa_exec_index,
            p_buffer,
            i_buffer_size,
            segment_info.f_duration,
            b_discontinuity);
    //goalbit_piece = v3;
    if ( this->piece_count > 11 )
    {
        if ( this->pGoalBitPiece[0] )
        {
            delete this->pGoalBitPiece[0];
            //operator delete(v4);
        }
        for ( i = 1; i <= 11; ++i ) {
            //*((_DWORD * ) & this->content_id + i - 1 + 8) = *((_DWORD * ) & this->content_id + i + 8);
            this->pGoalBitPiece[i-1] = this->pGoalBitPiece[i];
        }
        this->pGoalBitPiece[11] = goalbit_piece;
    }
    else
    {
        //*((_DWORD *)&this->content_id + this->buffer_count++ + 8) = v3;
        this->pGoalBitPiece[this->piece_count] = goalbit_piece;
        this->piece_count++;
    }
    this->i_gbtpv2d_last_quality = segment_info.i_quality;
}

session_t GoalBitSession::GetSessionType() {
    return this->session_type;
}

void goalbit_param_default(goalbit_param_t *param)
{
    //unsigned int v1; // eax@2

    if ( param )
    {
        param->b_verbose = 0;
        param->b_log_to_file = 0;
        param->i_verbose_level = 0;
        param->i_cache_size = 0x1000000;
        param->i_buffer_size = 8;
        param->i_urgent_size = 0xC;
        param->i_window_length = 0x18;
        param->f_exponential_beta = 0x3FC00000;
        param->i_max_peers_from_tracker = 100;
        param->i_max_leecher_list = 30;
        param->i_max_seed_list = 0;
        param->i_min_peers_list = 1;
        param->i_max_leecher_unchoke = 4;
        param->i_max_seeder_unchoke = 0;
        param->i_listen_port = 0;
        param->i_max_bandwidth_down = 0;
        param->i_max_bandwidth_up = 0;
        param->psz_p2p_tracker_url = 0;
        param->psz_hls_server_url = 0;
        param->l_listen_ip = inet_addr("0.0.0.0");
        param->psz_meta_server_url = 0;
        param->psz_content_id = 0;
        param->psz_base_dir = 0;
        param->psz_log_dir = 0;
        param->b_listen_for_connections = 1;
        param->b_disable_upnp = false;
        //param->i_closure_type = 1;
        param->i_closure_type = 2;
        //param->pf_event_callback = 0;
        param->p_external_ref = 0;
    }
}

/*
 void* goalbit_event_callback(goalbit_t *p_this, goalbit_event_t *p_event)
 {
 GoalBitSession *p_session; // ST20_4@5
 int percentage; // [sp+1Ch] [bp-1Ch]@3

 switch (p_event->i_event) {
 case 0:
 p_session = p_event->p_ref;
 p_session->SetBufferingState(false);
 return 0;
 case 1:
 return 0;
 case 2:
 percentage = p_event->g_value.i_int;
 if (percentage > 100) {
 percentage = 100;
 }
 p_session = p_event->p_ref;
 p_session->SetBufferingState(true);
 p_session->SetBufferingPercentage(percentage);
 return 0;
 case 3:
 p_event->p_ref->SetQualitiesDesc(p_event->g_value.goalbit_qualities_desc_t_01);
 return 0;
 case 4:
 return 0;
 }
 return 0;
 //    if ( p_event->i_event != 1 )
 //    {
 //        if ( p_event->i_event == 2 )
 //        {
 //            percentage = p_event->g_value.i_int;
 //            if ( percentage > 100 ) {
 //                percentage = 100;
 //            }
 //            p_session = p_event->p_ref;
 //            p_session->SetBufferingState( 1 );
 //            p_session->SetBufferingPercentage(percentage);
 //        } else if ( p_event->i_event ) {
 //            if ( p_event->i_event == 3 ) {
 //                p_event->p_ref->SetQualitiesDesc(p_event->g_value.goalbit_qualities_desc_t_01);
 //            }
 //        }
 //        else
 //        {
 //            p_event->p_ref->SetBufferingState( 0 );
 //        }
 //    }
 //    return 0;
 }
 */

int goalbit_param_check(goalbit_param_t_0 *param)
{
    char *v1; // eax@2
    char *v2; // eax@4
    char *v3; // eax@6
    char *v4; // esi@8
    //int v5; // eax@10
    //char *v6; // edi@10
    int result; // eax@10

    if ( param )
    {
        v1 = param->psz_p2p_tracker_url;
        if ( v1 && *v1 )
        {
            v2 = param->psz_hls_server_url;
            if ( v2 && *v2 )
            {
                v3 = param->psz_content_id;
                if ( v3 && *v3 )
                {
                    v4 = param->psz_base_dir;
                    if ( v4 && *v4 )
                    {
                        //v5 = strlen(param->psz_base_dir);
                        //v6 = (char *)malloc(v5 + 0xA);
                        //sprintf(v6, "mkdir -p %s", v4);
                        //system(v6);
                        //free(v6);
                        mkdir(param->psz_base_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                        param->i_max_bandwidth_down *= 125;
                        param->i_max_bandwidth_up *= 125;
                        result = 1;
                    }
                    else
                    {
                        print_err(__func__, "\n\nNo Base directory parameter found\n\n");
                        result = -5;
                    }
                }
                else
                {
                    print_err(__func__, "\n\nNo Content ID parameter found\n\n");
                    result = -4;
                }
            }
            else
            {
                print_err(__func__, "\n\nNo HLS server parameter found\n\n");
                result = -3;
            }
        }
        else
        {
            print_err(__func__, "\n\nNo Tracker parameter found\n\n");
            result = -2;
        }
    }
    else
    {
        print_err(__func__, "\n\nNo parameters found\n\n");
        result = -1;
    }
    return result;
}

int ares_library_init(int a1)
{
    if ( !ares_initialized )
    {
        ares_initialized = 1;
        ares_init_flags = a1;
    }
    return 0;
}

int Curl_resolver_global_init()
{
    return ares_library_init(1) >= 1 ? 2 : 0;
}


int Curl_srand()
{
    unsigned int result; // eax@1

    result = 0x807DBCB5 * time(0) - 0x58FBD821;
    randseed = result;
    return result;
}

int curl_global_init(int a1)
{
    int v1; // edx@1
    signed int result; // eax@1
    int v3; // edx@3

    v1 = initialized++;
    result = 0;
    if ( !v1 )
    {
        Curl_cmalloc = malloc;
        Curl_cfree = free;
        Curl_crealloc = realloc;
        Curl_cstrdup = strdup;
        Curl_ccalloc = calloc;
        v3 = Curl_resolver_global_init();
        result = 2;
        if ( !v3 )
        {
            init_flags = a1;
            Curl_srand();
            result = 0;
        }
    }
    return result;
}

void httpGlobalInit()
{
    curl_global_init(3);
}

void msleep(mtime_t delay)
{
    timeval tv_delay; // [sp+28h] [bp-14h]@1

    tv_delay.tv_sec = delay / 1000000;
    tv_delay.tv_usec = (int)delay % 1000000;
    select(0, 0, 0, 0, &tv_delay);
}

bool Closure_condition(goalbit_t_0 *p_this)
{
    int v1; // eax@1
    //goalbit_structure_t *v3; // eax@2
    HLSManager *v4; // edx@3
    bool v5; // dl@5
    bool result; // al@5
    btTracker *v7; // eax@8
    btTracker *v8; // edx@10

    v1 = p_this->p_param->i_closure_type;
    if ( v1 == 1 )
    {
        v7 = p_this->p_structure->Tracker;
        if ( v7 )
            result = (v7->_trackerF46 & 3) == 3;
        else
            result = !p_this->p_structure->b_btc_alive;
        return result;
    }
    //v3 = p_this->p_structure;
    if (v1 != 2) {
        v4 = p_this->p_structure->p_hlsManager;
        if ( v4 )
        {
            if ( p_this->p_structure->Tracker )
            {
                v5 = v4->isReady();
                result = 0;
                if ( !v5 )
                    result = (p_this->p_structure->Tracker->_trackerF46 & 3) == 3;
            }
            else
            {
                result = !v4->isReady();
            }
            return result;
        }
        v8 = p_this->p_structure->Tracker;
        if ( v8 ) {
            return (v8->_trackerF46 & 3) == 3;
        }
    }
    return !p_this->p_structure->b_btc_alive;
}

void Downloader(goalbit_t *p_this)
{
    int max_socket; // eax@20
    time_t v10; // eax@24
    suseconds_t v11; // edx@24
    BasicPeer *v13; // eax@29
    time_t v14; // ebx@30
    bool b_check_win; // bl@36
    double maxsleep; // [sp+50h] [bp-24Ch]@19
    fd_set rfd; // [sp+74h] [bp-228h]@18
    fd_set rfdnext; // [sp+F4h] [bp-1A8h]@1
    fd_set wfd; // [sp+174h] [bp-128h]@18
    fd_set wfdnext; // [sp+1F4h] [bp-A8h]@1
    timeval timeout; // [sp+274h] [bp-28h]@25

    int nfds = 0;
    FD_ZERO(&rfdnext);
    FD_ZERO(&wfdnext);

    bool b_hls_started = false;
    bool b_poll = false;
    int stopped = 0;
    do {
        time(&p_this->p_structure->now);
        if ( !p_this->p_structure->b_btc_alive ) {
            if ( stopped ) {
                continue;
            }
            if ( p_this->p_structure->Tracker )
            {
                p_this->p_structure->Tracker->SetStoped();
                p_this->p_structure->b_p2p_enabled = false;
                print_msg(__func__, "[Downloader] Stopping...");
                stopped = 1;
                continue;
            }
            if ( p_this->p_structure->p_hlsRate )
            {
                p_this->p_structure->p_hlsRate->reset();
            }
        }
        if ( !p_this->p_structure->p_hlsManager->isReady() ) {
            msleep(50000);
            continue;
        }
        if ( b_hls_started == false ) {
            print_msg(__func__, "[DOWNLOADER] HLS Manager ready! Start downloading...");
            p_this->p_structure->BTCONTENT->InitHLSExecution();
            p_this->p_structure->WORLD = new PeerList(p_this);
            p_this->p_structure->Self = new BasicPeer(p_this);
            p_this->p_structure->IPQUEUE = new IpList();// *)operator new(0xCu);
            p_this->p_structure->IPQUEUE->p_this = p_this;
            p_this->p_structure->IPQUEUE->ipl_head = 0;
            p_this->p_structure->IPQUEUE->count = 0;
            p_this->p_structure->PENDINGQUEUE = new PeerPendingQueue(p_this);
            p_this->p_structure->WORLD->Initial_ListenPort();
            p_this->p_structure->Tracker = new btTracker(p_this);
            p_this->p_structure->Tracker->Initial();
            p_this->p_structure->Tracker->IntervalCheck();
            if ( p_this->p_structure->psz_p2p_metadata_url ) {
                p_this->p_structure->p_metadataManager = new MetadataManager(p_this);
            }
            b_hls_started = true;
        } else {
            if ( p_this->p_structure->b_p2p_enabled == false ) {
                if ( p_this->p_structure->Tracker && p_this->p_structure->Tracker->_trackerF46 & 3 ) {
                    p_this->p_structure->Tracker->IntervalCheck();
                } else if ( p_this->p_structure->psz_p2p_metadata_url ) {
                    if ( !p_this->p_structure->p_metadataManager )
                    {
                        p_this->p_structure->p_metadataManager = new MetadataManager(p_this);
                    }
                    if ( p_this->p_structure->p_metadataManager->isReady() )
                    {
                        p_this->p_structure->BTCONTENT->InitP2PExecution();
                    }
                }
            } else {
                if ( mdate() > p_this->p_structure->i_last_bitfield_update + 30000000 ) {
                    p_this->p_structure->BTCONTENT->BitFieldP2PUpdate();
                    p_this->p_structure->i_last_bitfield_update = mdate();
                    print_msg(goalbitp2p,
                              "[DOWNLOADER] Bitfield synchronized! HLS_window[%d,%d], P2P_window[%d,%d]",
                              p_this->p_structure->i_hls_win_offset,
                              p_this->p_structure->i_hls_win_offset + p_this->p_structure->i_hls_win_length,
                              p_this->p_structure->i_p2p_win_offset,
                              p_this->p_structure->i_p2p_win_offset + p_this->p_structure->i_p2p_win_length);
                }
            }
        }

        /**
         * write file
         */
        p_this->p_structure->BTCONTENT->ProcessDownloadedHLSSegments();
        p_this->p_structure->BTCONTENT->ProcessDownloadedHLSPieces();
        p_this->p_structure->BTCONTENT->ExecutionMonitor();

        if ( p_this->p_structure->b_p2p_enabled ) {
            rfd = rfdnext;
            wfd = wfdnext;
            if ( b_poll )
            {
                FD_ZERO(&rfd);
                FD_ZERO(&wfd);
                maxsleep = 0.0;
            }
            else
            {
                p_this->p_structure->Self->p_rate_ul->_bf40 &= 0b11111110u;

                p_this->p_structure->Self->p_rate_dl->_bf40 &= 0b11111110u;
                maxsleep = -1.0;
            }
            p_this->p_structure->Tracker->IntervalCheck();
            max_socket = p_this->p_structure->WORLD->IntervalCheck(&rfd, &wfd);
            if ( max_socket < 0 )
                max_socket = -1;
            if ( !b_poll )
            {
                time(&p_this->p_structure->now);
                while ( p_this->p_structure->BTCONTENT->p_cache->FlushNeeded() && p_this->p_structure->WORLD->IsIdle() )
                {
                    p_this->p_structure->BTCONTENT->p_cache->FlushOldest();
                    time(&p_this->p_structure->now);
                    maxsleep = 0.0;
                }
            }
            rfdnext = rfd;
            wfdnext = wfd;
            if ( maxsleep < 0.0 )
            {
                maxsleep = p_this->p_structure->WORLD->WaitBW();
                if ( maxsleep > -100.0 )
                {
                    if (maxsleep <= 0)
                    {
                        v11 = 0;
                        v10 = 1;
                        maxsleep = 1.0;
                    }
                    else if ( maxsleep > 1.0 )
                    {
                        v11 = 0;
                        v10 = 1;
                        maxsleep = 1.0;
                    }
                    else
                    {
                        v10 = maxsleep;
                        v11 = ((maxsleep - (long double)(signed int)maxsleep) * 1000000.0);
                    }
                }
                else
                {
                    v11 = 0;
                    v10 = 0;
                    maxsleep = 0.0;
                }
            }
            else if ( maxsleep > 1.0 )
            {
                v11 = 0;
                v10 = 1;
                maxsleep = 1.0;
            }
            else
            {
                v10 = (signed int)maxsleep;
                v11 = (signed int)((maxsleep - (long double)(signed int)maxsleep) * 1000000.0);
            }
            timeout.tv_sec = v10;
            timeout.tv_usec = v11;
            p_this->p_structure->WORLD->_peerlistF9e &= 0b11100111;

            if (max_socket == -1) {
                FD_ZERO(&rfdnext);
                FD_ZERO(&wfdnext);
                nfds = 0;
            } else {
                nfds = select(max_socket + 1, &rfd, &wfd, 0, &timeout);
                if (nfds < 0) {
                    print_err(goalbitp2p, "Network Error: %d [timeout: %1d%1d]", errno, timeout.tv_sec, timeout.tv_usec);
                }
            }
            if ( !b_poll )
            {
                if ( nfds <= 0 )
                {
                    if ( maxsleep > 0.0) {
                        b_poll = (maxsleep < 1.0) ? true : false;
                    }
                }
                else
                {
                    v13 = p_this->p_structure->Self;
                    v13->p_rate_ul->_bf40 &= 0b11111110;
                    v13->p_rate_dl->_bf40 &= 0b11111110;
                }
            } else {
                b_poll = false;
            }
            v14 = p_this->p_structure->now;
            time(&p_this->p_structure->now);
            if ( p_this->p_structure->now == v14 - 1 )
                p_this->p_structure->now = v14;
            p_this->p_structure->WORLD->AnyNewPeer();
            if ( nfds > 0 ) {
                p_this->p_structure->WORLD->AnyPeerReady(&rfd, &wfd, &nfds, &rfdnext, &wfdnext);
            }
            while ( p_this->p_structure->BTCONTENT->p_cache->FlushNeeded() )
            {
                p_this->p_structure->BTCONTENT->p_cache->FlushOldest();
                time(&p_this->p_structure->now);
            }
        }
        Mypthread_mutex_lock(2, &p_this->p_structure->exec_lock);
        b_check_win = p_this->p_structure->b_check_win;
        Mypthread_mutex_unlock(2, &p_this->p_structure->exec_lock);
        if ( b_check_win ) {
            p_this->p_structure->BTCONTENT->WindowCheck();
        }
        msleep(50000);
    } while ( !Closure_condition(p_this));
    return;
}

void* goalbit_run(void* p)
{
    goalbit_t *p_data = (goalbit_t*)p;
    void *end; // [sp+1Ch] [bp-10h]@5

    srandom(mdate());
    print_msg(__func__, "\nLibGBSP is running...........");
    Downloader(p_data);
    if ( p_data->p_param->i_cache_size ) {
        p_data->p_structure->BTCONTENT->p_cache->FlushAll();
    }
    if ( p_data->p_structure->WORLD ) {
        p_data->p_structure->WORLD->CloseAll();
    }
    pthread_exit(&end);
    return 0;
}

goalbit_t *goalbit_client_open(goalbit_param_t *param)
{
    char *v4; // eax@4
    bool *v6; // edx@6
    goalbit_param_t *v7; // eax@6
    char *v8; // edx@8
    FILE *v9; // eax@8
    tm* v10; // eax@10
    goalbit_param_t *v11; // eax@10
    char *v12; // ebx@10
    char *v13; // eax@11
    char *v16; // eax@17
    char *v29; // eax@23
    int v30; // eax@23
    char *v34; // eax@25
    time_t v46; // [sp+30h] [bp-3Ch]@10
    char *s1; // [sp+34h] [bp-38h]@10
    char v48[0x14]; // [sp+38h] [bp-34h]@10

    if ( !param || goalbit_param_check(param) < 0 )
    {
        return 0;
    }
    /**
     * curl init
     */
    httpGlobalInit();

    /**
     * goalbit init, goalbitstructure init
     */
    goalbit_t* v1 = new goalbit_t();
    v1->p_structure = new goalbit_structure_t();
    memset(v1->p_structure, 0, sizeof(goalbit_structure_t));
    v1->p_param = param;
    v1->p_structure->psz_log_file = 0;
    v1->p_structure->p_log_file = 0;

    /**
     * for log file
     */
    if ( param->b_log_to_file == true ) {
        v4 = param->psz_log_dir;
        if ( !v4 || !*v4 ) {
            v6 = &v1->p_structure->b_btc_alive;
            //v45 = v1->p_structure;
            param->psz_log_dir = strdup(param->psz_base_dir);
            v7 = v1->p_param;
            v9 = 0;
            if ( v7 && v6 ) {
                v8 = v7->psz_log_dir;
                if ( v8 && *v8 )
                {
                    v46 = time(0);
                    v10 = localtime(&v46);
                    strftime(v48, sizeof(v48), "%Y%m%d_%H", v10);
                    v11 = v1->p_param;
                    s1 = 0;
                    asprintf(&s1, "%s/libgoalbit_%s.log", v11->psz_log_dir, v48);
                    v12 = s1;
                    if ( *s1 )
                    {
                        v13 = s1;
                        do
                            ++v13;
                        while ( *v13 );
                    }
                    //v14 = v1->p_structure;
                    if ( v1->p_structure->psz_log_file && !strcmp(s1, v1->p_structure->psz_log_file) )
                    {
                        free(v12);
                        v9 = v1->p_structure->p_log_file;
                    }
                    else
                    {
                        if ( v1->p_structure->p_log_file )
                        {
                            fflush(v1->p_structure->p_log_file);
                            fclose(v1->p_structure->p_log_file);
                            //v15 = v1->p_structure;
                            v1->p_structure->p_log_file = 0;
                            free(v1->p_structure->psz_log_file);
                            //v14 = v1->p_structure;
                            v12 = s1;
                            v1->p_structure->psz_log_file = 0;
                        }
                        v16 = strdup(v12);
                        //v17 = v1->p_structure;
                        v1->p_structure->psz_log_file = v16;
                        v1->p_structure->p_log_file = fopen(s1, "w+a");
                        free(s1);
                        v9 = v1->p_structure->p_log_file;
                    }
                }
            }
            v1->p_structure->p_log_file = v9;
        }
    }
    print_msg(__func__, "\n\nOPENING LibGBSP...\n\n");
    //v18 = v1->p_structure;
    v1->p_structure->b_buffering = true;
    v1->p_structure->b_p2p_syncing = true;
    v1->p_structure->i_p2p_sync_start = mdate();
    //v19 = v1->p_structure;
    v1->p_structure->exec_info.f_segment_played = 0.0;
    v1->p_structure->exec_info.i_quality = 0x80000001;
    v1->p_structure->exec_info.f_abd = 0.0;
    v1->p_structure->exec_info.i_segment_id = 0x80000001;
    v1->p_structure->exec_info.i_segment_offset = 0;
    v1->p_structure->exec_info.b_segment_completed = false;
    v1->p_structure->exec_info.i_played_time = 0;
    v1->p_structure->exec_info.i_abi = 0x80000001;
    v1->p_structure->i_target_quality = 0;
    v1->p_structure->b_fixed_quality = false;
    //v20 = mdate();
    v1->p_structure->i_last_buff_start = mdate();
    //v23 = v1->p_structure;
    if ( param->i_buffer_size > 0 )
        v1->p_structure->f_hls_buffer_size = param->i_buffer_size;
    else
        v1->p_structure->f_hls_buffer_size = 8;
    v1->p_structure->f_qoe = 0.0;
    if ( param->i_urgent_size > 0 )
        v1->p_structure->f_hls_urgent_size = param->i_urgent_size;
    else
        v1->p_structure->f_hls_urgent_size = 0x0C;
    v1->p_structure->i_buff_num = 0;
    v1->p_structure->i_startup_buff_dur = 0;
    v1->p_structure->i_total_rebuff_dur = 0;
    v1->p_structure->b_hls_live = 0;
    v1->p_structure->i_hls_qualities = 0;
    v1->p_structure->i_hls_win_offset = 0x80000001;
    v1->p_structure->i_hls_win_length = param->i_window_length;
    v1->p_structure->b_hls_can_down_pieces = true;
    v1->p_structure->b_btc_alive = true;
    v1->p_structure->i_check_port_opened = 1;
    v1->p_structure->i_tracker_view_port_opened = 1;
    v1->p_structure->i_last_exec_report = mdate();
    v1->p_structure = v1->p_structure;
    v1->p_structure->i_last_bitfield_update = mdate();
    //v28 = v1->p_structure;
    v29 = param->psz_meta_server_url;
    v1->p_structure->f_avg_hls_bitrate = 0;
    v1->p_structure->psz_p2p_metadata_url = v29;
    v30 = param->i_window_length;
    v1->p_structure->f_avg_hls_length = 0;
    v1->p_structure->f_avg_p2p_bitrate = 0;
    v1->p_structure->f_avg_p2p_length = 0;
    v1->p_structure->i_last_sync_date = 0;
    v1->p_structure->i_last_quality_change = 0;
    v1->p_structure->i_quality_meter = 0;
    v1->p_structure->i_history_count = 0;
    v1->p_structure->f_avg_usec_x_segment = 0x4A742400;
    v1->p_structure->f_avg_usec_x_piece = 0x49742400;
    v1->p_structure->f_avg_segment_x_sec = 0x3F800000;
    v1->p_structure->b_p2p_enabled = false;
    v1->p_structure->i_p2p_win_offset = 0x80000001;
    v1->p_structure->i_p2p_win_length = 5 * v30;
    v1->p_structure->i_eos_piece_id = 0x80000001;
    v1->p_structure->b_eos_event_sent = 0;
    v1->p_structure->b_check_win = false;
    v1->p_structure->cfg_req_queue_length = 3;
    v1->p_structure->cfg_max_listen_port = 3706;
    v1->p_structure->cfg_min_listen_port = 2706;
    v1->p_structure->arg_user_agent = (char*)malloc(9);
    strcpy(v1->p_structure->arg_user_agent, "-CD0302-");
    //*(_DWORD *)v31 = 0x3044432D;
    //*(_DWORD *)(v31 + 4) = 0x2D323033;
    //*(_BYTE *)(v31 + 8) = 0;
    v1->p_structure->cfg_user_agent = (char *)malloc(0x0E);
    sprintf(v1->p_structure->cfg_user_agent, "%s/%s", "LIBGBSP", "1.0.0");
    while ( true ) {
        //v33 = v1->p_structure;
        v34 = strchr(v1->p_structure->cfg_user_agent, ' ');
        if ( !v34 )
            break;
        *v34 = '-';
    }
    v1->p_structure->g_next_up = 0;
    v1->p_structure->g_next_dn = 0;
    v1->p_structure->now = time(0);
    pthread_mutex_init(&v1->p_structure->exec_lock, 0);
    //v35 = new HLSManager(v1);
    //v35 = (HLSManager *)operator new(0x54u);
    //HLSManager::HLSManager(v35, v1);
    //v36 = v1->p_structure;
    /**
     * goalbitstructure member new HLSManager, new BWRate, new btContent
     */
    v1->p_structure->p_hlsManager = new HLSManager(v1);
    v1->p_structure->p_metadataManager = 0;
    //v37 = (BWRate *)operator new(0x40u);
    //BWRate::BWRate(v37);
    //v38 = v1->p_structure;
    v1->p_structure->p_hlsRate = new BWRate();
    v1->p_structure->Self = 0;
    //v39 = (btContent *)operator new(0x94u);
    //btContent::btContent(v39, v1);
    //v40 = v1->p_structure;
    v1->p_structure->BTCONTENT = new btContent(v1);
    v1->p_structure->IPQUEUE = 0;
    v1->p_structure->Tracker = 0;
    v1->p_structure->WORLD = 0;
    v1->p_structure->PENDINGQUEUE = 0;

    /**
     * Downloader thread
     */
    pthread_mutex_init(&v1->client_lock, 0);
    v1->p_client = new pthread_t();
    if ( pthread_create(v1->p_client, 0, goalbit_run, (void*)v1) )
    {
        print_msg(__func__, "pthread_create failed: can't run LibGBSP.");
        v1->p_client = 0;
    }
    print_msg(__func__, "\n\nLibGBSP Running...\n\n");
    return v1;
}

void goalbit_param_print(goalbit_t_0 *p_this, goalbit_param_t_0 *param)
{
    if ( p_this ) {
        if ( param ) {
            print_msg(__func__, " [PARAM] i_cache_size = %d", param->i_cache_size);
            print_msg(__func__, " [PARAM] i_buffer_size = %d", param->i_buffer_size);
            print_msg(__func__, " [PARAM] i_urgent_size = %d", param->i_urgent_size);
            print_msg(__func__, " [PARAM] i_window_length = %d", param->i_window_length);
            print_msg(__func__, " [PARAM] f_exponential_beta = %f", param->f_exponential_beta);
            print_msg(__func__, " [PARAM] i_max_peers_from_tracker = %d", param->i_max_peers_from_tracker);
            print_msg(__func__, " [PARAM] i_max_leecher_list = %d", param->i_max_leecher_list);
            print_msg(__func__, " [PARAM] i_max_seed_list = %d", param->i_max_seed_list);
            print_msg(__func__, " [PARAM] i_min_peers_list = %d", param->i_min_peers_list);
            print_msg(__func__, " [PARAM] i_max_leecher_unchoke = %d", param->i_max_leecher_unchoke);
            print_msg(__func__, " [PARAM] i_max_seeder_unchoke = %d", param->i_max_seeder_unchoke);
            print_msg(__func__, " [PARAM] l_listen_ip = %ld", param->l_listen_ip);
            print_msg(__func__, " [PARAM] i_listen_port = %d", param->i_listen_port);
            print_msg(__func__, " [PARAM] i_max_bandwidth_down = %d", param->i_max_bandwidth_down);
            print_msg(__func__, " [PARAM] i_max_bandwidth_up = %d", param->i_max_bandwidth_up);
            print_msg(__func__, " [PARAM] psz_p2p_tracker_url = %s", param->psz_p2p_tracker_url);
            print_msg(__func__, " [PARAM] psz_hls_server_url = %s", param->psz_hls_server_url);
            print_msg(__func__, " [PARAM] psz_meta_server_url = %s", param->psz_meta_server_url);
            print_msg(__func__, " [PARAM] psz_content_id = %s", param->psz_content_id);
            print_msg(__func__, " [PARAM] psz_base_dir = %s", param->psz_base_dir);
            print_msg(__func__, " [PARAM] psz_log_dir = %s", param->psz_log_dir);
            print_msg(__func__, " [PARAM] b_listen_for_connections = %d", param->b_listen_for_connections);
            print_msg(__func__, " [PARAM] b_disable_upnp = %d", param->b_disable_upnp);
            print_msg(__func__, " [PARAM] i_closure_type = %d", param->i_closure_type);
        }
        else
        {
            print_msg(__func__, "Empty params!!");
        }
    }
}

GoalBitSession::GoalBitSession(int _session_id,
                               session_t _session_type,
                               char *_content_id,
                               char *_p2p_tracker_url,
                               char *_hls_server_url,
                               char *_p2p_manifest_url,
                               int _start_buffer_size,
                               int _urgent_buffer_size)
{
    print_msg(__func__, " === GoalBitSession ===");
    print_msg(__func__, " = _session_id         = %d", _session_id);
    print_msg(__func__, " = _session_type       = %d", _session_type);
    print_msg(__func__, " = _content_id         = %s", _content_id);
    print_msg(__func__, " = _tracker            = %s", _p2p_tracker_url);
    print_msg(__func__, " = _server             = %s", _hls_server_url);
    if ( _p2p_manifest_url ) {
        print_msg(__func__, " = _p2p_manifest_url   = %s", _p2p_manifest_url);
    }
    print_msg(__func__, " = _start_buffer_size  = %d", _start_buffer_size);
    print_msg(__func__, " = _urgent_buffer_size = %d", _urgent_buffer_size);
    this->session_id = _session_id;
    this->session_type = _session_type;
    this->content_id = _content_id;
    this->p2p_tracker_url = _p2p_tracker_url;
    this->hls_server_url = _hls_server_url;
    this->p2p_manifest_url = 0;
    if ( _p2p_manifest_url )
        this->p2p_manifest_url = _p2p_manifest_url;
    this->start_buffer_size = _start_buffer_size;
    this->urgent_buffer_size = _urgent_buffer_size;
    this->piece_count = 0;
    this->last_used_time = time(0);
    this->next_read_time = 0LL;
    this->i_gbtpv2d_buff_perc = 0;
    this->b_gbtpv2d_buffering = true;
    this->i_gbtpv2d_last_quality = 0x80000000;
    //this->b_gpa_buffering = true;
    this->b_gpa_prebuffered = false;
    this->i_gpa_exec_index = 0;
    this->p_qualities_desc = 0;
    for (int i = 0; i <= 11; ++i ) {
        pGoalBitPiece[i] = 0;//*((_DWORD * ) & this->content_id + i + 8) = 0;
    }

    /**
     * set goalbit param
     */
    this->p_param = new goalbit_param_t();
    goalbit_param_default(this->p_param);
    //v9 = this->content_id;
    asprintf(&this->p_param->psz_content_id, "%s", this->content_id);
    //v10 = this->p2p_tracker_url;
    asprintf(&this->p_param->psz_p2p_tracker_url, "%s", this->p2p_tracker_url);
    //v11 = this->hls_server_url;
    asprintf(&this->p_param->psz_hls_server_url, "%s", this->hls_server_url);
    if ( this->p2p_manifest_url )
    {
        //v12 = this->p2p_manifest_url;
        asprintf(&this->p_param->psz_meta_server_url, "%s", this->p2p_manifest_url);
    }
    //this->p_param->pf_event_callback = &goalbit_event_callback;
    this->p_param->p_external_ref = this;
    if ( libgbsp_base_dir && *libgbsp_base_dir ) {
        //v13 = this->session_id;
        asprintf(&this->p_param->psz_base_dir, "%s%c%d", libgbsp_base_dir, '/', this->session_id);
    } else {
        //v14 = this->session_id;
        asprintf(&this->p_param->psz_base_dir, ".%cbase_dir%c%d", '/', '/', this->session_id);
    }
    if ( libgbsp_log_dir ) {
        if ( *libgbsp_log_dir ) {
            this->p_param->psz_log_dir = strdup(libgbsp_log_dir);
            this->p_param->b_log_to_file = 1;
        }
        this->p_param->b_verbose = 1;
        this->p_param->i_verbose_level = 3;
    } else {
        this->p_param->i_verbose_level = -1;
    }
    if ( this->start_buffer_size > 0 )
        this->p_param->i_buffer_size = this->start_buffer_size;
    if ( this->urgent_buffer_size > 0 )
        this->p_param->i_urgent_size = this->urgent_buffer_size;

    /**
     * generate goalbit from goalbit param
     */
    this->p_goalbit = goalbit_client_open(this->p_param);
    if ( this->p_goalbit ) {
        goalbit_param_print(this->p_goalbit, this->p_param);
    } else {
        print_msg(__func__, "Could not open libgoalbit library");
    }
    return;
}

void GoalBitSession::SetBufferingState(bool b_buff) {
    b_gbtpv2d_buffering = b_buff;
    if ( b_buff )
    {
        piece_count = 0;
        next_read_time = 0LL;
        //b_gpa_buffering = false;
    }
}

void GoalBitSession::SetBufferingPercentage(int percentage) {
    i_gbtpv2d_buff_perc = percentage;
}

void GoalBitSession::SetQualitiesDesc(goalbit_qualities_desc_t_0 *p_qualities)
{
    p_qualities_desc = p_qualities;
}

goalbit_qualities_desc_t_0 *GoalBitSession::GetQualitiesDesc() {
    return p_qualities_desc;
}

int GoalBitSession::GetBufferingPercentage() {
    return i_gbtpv2d_buff_perc;
}

int goalbit_client_control(goalbit_t_0 *p_this, int i_action, goalbit_value_t *p_value)
{
    signed int result; // eax@1
    //goalbit_structure_t *v4; // eax@9
    unsigned int v5; // ecx@9

    result = 0;
    if ( p_this )
    {
        if ( i_action == 1 )
        {
            if ( p_value )
            {
                //v4 = p_this->p_structure;
                v5 = p_value->i_int;
                if ( p_value->i_int >= p_this->p_structure->i_hls_qualities || v5 >> 0x1F )
                {
                    p_this->p_structure->b_fixed_quality = false;
                    result = 1;
                }
                else
                {
                    p_this->p_structure->b_fixed_quality = true;
                    p_this->p_structure->i_target_quality = v5;
                    result = 1;
                }
            }
        }
        else if ( i_action == 2 )
        {
            if ( p_value )
            {
                p_value->i_int = p_this->p_structure->i_target_quality;
                result = 1;
            }
        }
        else
        {
            result = 1;
        }
    }
    return result;
}

void GoalBitSession::SetQuality(int quality_id) {
    goalbit_value_t value; // [sp+1Ch] [bp-Ch]@1

    value.i_int = quality_id;
    goalbit_client_control(this->p_goalbit, 1, &value);
}

int GoalBitSession::GetQuality() {
    bool v1; // al@3
    //unsigned int result; // eax@6
    goalbit_value_t *p_value; // [sp+1Ch] [bp-Ch]@1

    p_value = new goalbit_value_t();
    v1 = goalbit_client_control(this->p_goalbit, 2, p_value) && p_value;
    if ( v1 )
        return p_value->i_int;
    return -1;
}

void httpGlobalCleanUP()
{
    curl_global_cleanup();
}

void goalbit_client_close(goalbit_t_0 *p_this)
{
    goalbit_structure_t *v1; // eax@1
    goalbit_structure_t *v2; // eax@1
    HLSManager *v3; // esi@7
    MetadataManager *v4; // esi@9
    BWRate *v5; // esi@11
    BasicPeer *v6; // esi@13
    btContent *v7; // esi@15
    IpList *v8; // esi@17
    btTracker *v9; // esi@21
    PeerList *v10; // esi@23
    PeerPendingQueue *v11; // esi@25
    goalbit_structure_t *v12; // eax@27
    void *end; // [sp+1Ch] [bp-10h]@1

    print_msg(__func__, "\n\nClosing LibGBSP...\n\n");
    pthread_mutex_lock(&p_this->p_structure->exec_lock);
    v1 = p_this->p_structure;
    v1->b_btc_alive = false;
    pthread_mutex_unlock(&v1->exec_lock);
    pthread_join(*p_this->p_client, &end);
    v2 = p_this->p_structure;
    if ( p_this->p_structure->arg_user_agent )
    {
        free(p_this->p_structure->arg_user_agent);
        v2 = p_this->p_structure;
        p_this->p_structure->arg_user_agent = 0;
    }
    if ( v2->cfg_user_agent )
    {
        free(v2->cfg_user_agent);
        v2 = p_this->p_structure;
        p_this->p_structure->cfg_user_agent = 0;
    }
    if ( v2->psz_p2p_metadata_url )
    {
        free(v2->psz_p2p_metadata_url);
        v2 = p_this->p_structure;
        p_this->p_structure->psz_p2p_metadata_url = 0;
    }
    v3 = v2->p_hlsManager;
    if ( v3 )
    {
        delete v2->p_hlsManager;
        v2 = p_this->p_structure;
    }
    v4 = v2->p_metadataManager;
    if ( v4 )
    {
        delete v2->p_metadataManager;
        v2 = p_this->p_structure;
    }
    v5 = v2->p_hlsRate;
    if ( v5 )
    {
        delete v2->p_hlsRate;
        v2 = p_this->p_structure;
    }
    v6 = v2->Self;
    if ( v6 )
    {
        delete v2->Self;
        v2 = p_this->p_structure;
    }
    v7 = v2->BTCONTENT;
    if ( v7 )
    {
        delete v2->BTCONTENT;
        v2 = p_this->p_structure;
    }
    v8 = v2->IPQUEUE;
    if ( v8 )
    {
        if ( v8->ipl_head ) {
            v2->IPQUEUE->_Emtpy();
        }
        delete v8;
        v2 = p_this->p_structure;
    }
    v9 = v2->Tracker;
    if ( v9 )
    {
        delete v2->Tracker;
        v2 = p_this->p_structure;
    }
    v10 = v2->WORLD;
    if ( v10 )
    {
        delete v2->WORLD;
        v2 = p_this->p_structure;
    }
    v11 = v2->PENDINGQUEUE;
    if ( v11 )
    {
        delete v2->PENDINGQUEUE;
    }
    free(p_this->p_client);
    v12 = p_this->p_structure;
    p_this->p_client = 0;
    pthread_mutex_destroy(&v12->exec_lock);
    pthread_mutex_destroy(&p_this->client_lock);
    httpGlobalCleanUP();
    print_msg(__func__, "\n\nLibGBSP Closed!\n\n");
    free(p_this->p_structure);
    free(p_this);
}

GoalBitSession::~GoalBitSession() {
    goalbit_t_0 *p_goalbit_aux; // ST1C_4@2
    //void *v2; // ebx@5
    int i; // [sp+14h] [bp-14h]@3
    unsigned int i_0; // [sp+18h] [bp-10h]@10

    print_err(__func__, "Close goalbit session\n");
    if ( this->p_goalbit )
    {
        p_goalbit_aux = this->p_goalbit;
        this->p_goalbit = 0;
        goalbit_client_close(p_goalbit_aux);
    }
    for ( i = 0; i < 12; ++i )
    {
        if ( pGoalBitPiece[i] )
        {
            delete pGoalBitPiece[i];
        }
    }
    if ( this->p_qualities_desc )
    {
        for ( i_0 = 0; this->p_qualities_desc->i_quality_num > i_0; ++i_0 )
        {
            if ( this->p_qualities_desc->p_qualities[i_0].psz_resolution )
                free(this->p_qualities_desc->p_qualities[i_0].psz_resolution);
        }
        free(this->p_qualities_desc->p_qualities);
        free(this->p_qualities_desc);
    }
    free(this->p_param->psz_content_id);
    free(this->p_param->psz_p2p_tracker_url);
    free(this->p_param->psz_hls_server_url);
    free(this->p_param->psz_base_dir);
    free(this->p_param);
}

int GoalBitManager::GetSessionIndex(int session_id)
{
    bool v2; // al@4
    int index; // [sp+18h] [bp-10h]@1
    int i; // [sp+1Ch] [bp-Ch]@1

    index = -1;
    for ( i = 0; this->sessions_count > i; ++i )
    {
        v2 = this->goalbit_sessions[i] && this->goalbit_sessions[i]->GetSessionId() == session_id;
        if ( v2 )
            return i;
    }
    return index;
}

void GoalBitManager::GetPieceContent(int session_id, int piece_id, uint8_t **piece_content, int *size)
{
    bool v5; // al@3
    int i; // [sp+1Ch] [bp-Ch]@1

    print_dbg(__func__, "Get piece content, session-id=(%d), piece-id=(%d)\n", session_id, piece_id);
    Mypthread_mutex_lock(3, &this->mutex);
    i = GetSessionIndex(session_id);
    v5 = i >= 0 && this->goalbit_sessions[i]->IsReady();
    if ( v5 )
        this->goalbit_sessions[i]->GetPieceContent(piece_id, piece_content, size);
    else
        *size = 0;
    Mypthread_mutex_unlock(3, &this->mutex);
}

bool GoalBitManager::CanPrebuffer(int session_id) {
    int i; // [sp+18h] [bp-10h]@1
    bool prebuffer; // [sp+1Fh] [bp-9h]@1

    prebuffer = 0;
    Mypthread_mutex_lock(4, &this->mutex);
    i = GetSessionIndex(session_id);
    if ( i >= 0 )
        prebuffer = this->goalbit_sessions[i]->CanPrebuffer();
    Mypthread_mutex_unlock(4, &this->mutex);
    return prebuffer;
}

goalbit_block_t *GoalBitManager::Prebuffer(int session_id, int *err) {
    bool v3; // al@3
    goalbit_block_t *p_content; // [sp+18h] [bp-10h]@1
    int i; // [sp+1Ch] [bp-Ch]@1

    print_msg(__func__, "Prebuffer pieces, session-id=(%d)\n", session_id);
    *err = -1;
    p_content = 0;
    Mypthread_mutex_lock(5, &this->mutex);
    i = GetSessionIndex(session_id);
    v3 = i >= 0 && this->goalbit_sessions[i]->IsReady();
    if ( v3 ) {
        p_content = this->goalbit_sessions[i]->Prebuffer(err);
    }
    Mypthread_mutex_unlock(5, &this->mutex);
    return p_content;
}

goalbit_block_t *GoalBitManager::GetNextPiece(int session_id, int *err) {
    bool v3; // al@3
    goalbit_block_t *p_content; // [sp+18h] [bp-10h]@1
    int i; // [sp+1Ch] [bp-Ch]@1

    print_msg(__func__, "Get next piece content, session-id=(%d)\n", session_id);
    *err = -1;
    p_content = 0;
    Mypthread_mutex_lock(6, &this->mutex);
    i = GetSessionIndex(session_id);
    v3 = i >= 0 && this->goalbit_sessions[i]->IsReady();
    if ( v3 )
        p_content = this->goalbit_sessions[i]->GetNextPiece(err);
    Mypthread_mutex_unlock(6, &this->mutex);
    return p_content;
}

void GoalBitManager::GetPieces(int session_id, int **pieces_ids, float **pieces_durations, bool **pieces_disconts, int *size, float *max_duration) {
    //print_msg(__func__, "Get pieces, session-id=(%d)\n", session_id);
    Mypthread_mutex_lock(7, &this->mutex);
    int i = GetSessionIndex(session_id);
    bool v7 = i >= 0 && this->goalbit_sessions[i]->IsReady();
    if ( v7 )
        this->goalbit_sessions[i]->GetPieces(
                pieces_ids,
                pieces_durations,
                pieces_disconts,
                size,
                max_duration);
    else {
        *size = 0;
    }
    Mypthread_mutex_unlock(7, &this->mutex);
}

int GoalBitManager::NewSession(session_t session_type, char *content_id, char *p2p_tracker_url,
                               char *hls_server_url, char *p2p_manifest_url, int start_buffer,
                               int urgent_buffer) {
    int result; // eax@2
    //GoalBitSession *v9; // ebx@3
    //GoalBitSession *v10; // ebx@6
    int session_id; // [sp+38h] [bp-10h]@3
    int i; // [sp+3Ch] [bp-Ch]@3

    // session_type : HTTP_LIVE_STREAMING
    // content_id : "npo1"
    // p2p_tracker_url : "http://vm01-hwnode03-rdm-nl.rdm.nl.infra.cymtv.net/goalbit-media-server/tracker/announce"
    // hls_server_url : "http://vm01-hwnode03-rdm-nl.rdm.nl.infra.cymtv.net/goalbit-media-server/hls/dGltZT0xNTA3MzQzMDAxJnNpZ249YTA1ZWI1ZGM2YWRhODA2MWE0ZWU1NTZkNzdhY2MwY2MmcHJlZml4PTUy/vm01-hwnode03-rdm-nl/npo1/playlist.m3u8"
    // p2p_manifest_url : "http://vm01-hwnode03-rdm-nl.rdm.nl.infra.cymtv.net/goalbit-media-server/manifest/p2p/npo1"
    if ( this->sessions_count <= 4 ) {
        Mypthread_mutex_lock(8, &this->mutex);
        session_id = this->global_sessions_id;
        i = this->sessions_count;
        print_msg(__func__, "  session_id  = %d", this->global_sessions_id);
        print_msg(__func__, "  session_pos = %d", i);
        //v9 = (GoalBitSession *)operator new(0x80u);
        this->goalbit_sessions[i] = new GoalBitSession(
                session_id,
                session_type,
                content_id,
                p2p_tracker_url,
                hls_server_url,
                p2p_manifest_url,
                start_buffer,
                urgent_buffer);
        if ( this->goalbit_sessions[i] ) {
            if ( this->goalbit_sessions[i]->IsOk() ) {
                ++this->global_sessions_id;
                ++this->sessions_count;
            } else {
                print_msg(__func__, "ERROR when openning goalbit session (2)\n");
                if ( goalbit_sessions[i] )
                {
                    //GoalBitSession::~GoalBitSession(this->goalbit_sessions[i]);
                    //operator delete(v10);
                    delete goalbit_sessions[i];
                }
                this->goalbit_sessions[i] = 0;
                session_id = 0;
            }
        } else {
            print_msg(__func__, "ERROR when openning goalbit session (1)\n");
            session_id = 0;
        }
        Mypthread_mutex_unlock(8, &this->mutex);
        if ( session_id ) {
            print_msg(__func__, "Session %d created\n", session_id);
        }
        result = session_id;
    } else {
        print_msg(__func__, "Max number of goalbit sessions reached\n");
        result = 0;
    }
    return result;
}

goalbit_qualities_desc_t_0 *GoalBitManager::GetQualitiesDesc(int session_id) {
    goalbit_qualities_desc_t_0 *p_qualities; // [sp+18h] [bp-10h]@1
    int i; // [sp+1Ch] [bp-Ch]@1

    p_qualities = 0;
    Mypthread_mutex_lock(9, &this->mutex);
    i = GetSessionIndex(session_id);
    if ( i >= 0 )
        p_qualities = this->goalbit_sessions[i]->GetQualitiesDesc();
    Mypthread_mutex_unlock(9, &this->mutex);
    return p_qualities;
}

int GoalBitManager::GetBufferingPercentage(int session_id) {
    int buf_percentage; // [sp+18h] [bp-10h]@1
    int i; // [sp+1Ch] [bp-Ch]@1

    buf_percentage = 0;
    Mypthread_mutex_lock(10, &this->mutex);
    i = GetSessionIndex(session_id);
    if ( i >= 0 )
        buf_percentage = this->goalbit_sessions[i]->GetBufferingPercentage();
    Mypthread_mutex_unlock(10, &this->mutex);
    return buf_percentage;
}

void GoalBitManager::SetSessionQuality(int session_id, int quality_id) {
    int i; // [sp+1Ch] [bp-Ch]@1

    Mypthread_mutex_lock(11, &this->mutex);
    i = GetSessionIndex(session_id);
    if ( i >= 0 )
        this->goalbit_sessions[i]->SetQuality(quality_id);
    Mypthread_mutex_unlock(11, &this->mutex);
}

int GoalBitManager::GetSessionQuality(int session_id) {
    int q; // [sp+18h] [bp-10h]@1
    int i; // [sp+1Ch] [bp-Ch]@1

    Mypthread_mutex_lock(12, &this->mutex);
    i = GetSessionIndex(session_id);
    q = 0xFFFFFFFF;
    if ( i >= 0 )
        q = this->goalbit_sessions[i]->GetQuality();
    Mypthread_mutex_unlock(12, &this->mutex);
    return q;
}

session_t GoalBitManager::GetSessionType(int session_id) {
    bool v2; // al@4
    session_t result; // eax@11
    int index; // [sp+18h] [bp-10h]@1
    int i; // [sp+1Ch] [bp-Ch]@1

    index = -1;
    for ( i = 0; this->sessions_count > i; ++i )
    {
        v2 = this->goalbit_sessions[i] && this->goalbit_sessions[i]->GetSessionId() == session_id;
        if ( v2 )
        {
            index = i;
            break;
        }
    }
    if ( index < 0 )
        result = NO_SESSION_TYPE;
    else
        result = this->goalbit_sessions[index]->GetSessionType();
    return result;
}

bool GoalBitManager::IsReady(int session_id) {
    int i; // [sp+18h] [bp-10h]@1
    bool ready; // [sp+1Fh] [bp-9h]@1

    ready = 0;
    Mypthread_mutex_lock(13, &this->mutex);
    i = GetSessionIndex(session_id);
    if ( i >= 0 ) {
        ready = this->goalbit_sessions[i]->IsReady();
    }
    Mypthread_mutex_unlock(13, &this->mutex);
    return ready;
}

GoalBitManager::~GoalBitManager() {
    GoalBitSession *v1; // ebx@3
    int i; // [sp+1Ch] [bp-Ch]@1

    print_msg(__func__, "End GoalBit Manager\n");
    this->running = 0;
    pthread_mutex_lock(&this->mutex);
    for ( i = 0; i <= 4; ++i )
    {
        if ( this->goalbit_sessions[i] )
        {
            v1 = this->goalbit_sessions[i];
            if ( v1 )
            {
                delete goalbit_sessions[i];
            }
            this->goalbit_sessions[i] = 0;
        }
    }
    pthread_mutex_unlock(&this->mutex);
    if ( this->control_thread )
    {
        pthread_join(*this->control_thread, 0);
        free(this->control_thread);
    }
    pthread_mutex_destroy(&this->mutex);
}

char* sub_3C60(mg_connection* v1)
{
    //int v1; // r6@1
    int v2; // r7@1
    //int result; // r0@1
    char *ptr; // [sp+0h] [bp-13A8h]@1
    char dest[5028]; // [sp+4h] [bp-13A4h]@1
    //int v6; // [sp+138Ch] [bp-1Ch]@1

    //v1 = a1;
    //v6 = _stack_chk_guard;
    dest[0] = 0;
    strcat(dest, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n");
    strcat(dest, "<cross-domain-policy>\r\n");
    strcat(dest, "<allow-access-from domain=\"*\"/>\r\n");
    strcat(dest, "</cross-domain-policy>");
    v2 = strlen(dest);
    asprintf(&ptr, "text/xml");
    sub_39B8(v1, ptr, v2);
    mg_printf(v1, "%s", dest);
    free(ptr);
    //result = dword_70C0;
    //if ( v6 != _stack_chk_guard )
    //    _stack_chk_fail(dword_70C0);
    return dword_70C0;
}

unsigned int goalbit_block_duration(block_t *p_block, int i_offset, int i_length)
{
    return MpegTS::getDuration(p_block, i_offset, i_length);
}

int64_t sub_3A28(int64_t a1, int a2)
{
    int64_t v2; // [sp+0h] [bp-18h]@1
    v2 = a1;
    while ( nanosleep((const struct timespec *)&v2, (struct timespec *)&v2) && errno == 4 )
        ;
    return v2;
}

void mg_event_callback(struct mg_connection *a2, int a1, void* p3)
//void* mg_event_callback(mg_event  a1, struct mg_connection *a2, const struct mg_request_info *a3)
{
    //void *v3; // r5@1
    //int v4; // r6@1
    const char *v5; // r4@2
    char* v6; // r0@4
    int v7; // r0@5
    char *v8; // r7@6
    char *v9; // r4@6
    const char *v10; // r4@6
    const char *v12; // r0@9
    int vPiece; // r0@10
    //int v15; // r0@12
    int v16; // r0@3
    //void *v17; // r7@16
    const char *v18; // r0@18
    int v19; // r0@19
    //int v20; // r5@21
    int v21; // r6@25
    int v22; // r5@30
    //int v24; // r0@32
    int v25; // r0@32
    int v26; // r1@32
    int v27; // r2@32
    int v28; // r0@17
    int v30; // [sp+14h] [bp-4Ch]@19
    //int v31; // [sp+18h] [bp-48h]@1
    int vSession; // [sp+1Ch] [bp-44h]@5
    int v33; // [sp+1Ch] [bp-44h]@29
    signed int v34; // [sp+20h] [bp-40h]@21
    unsigned int v35; // [sp+24h] [bp-3Ch]@32
    int v36; // [sp+28h] [bp-38h]@32
    uint8_t *v37; // [sp+2Ch] [bp-34h]@29
    char *needle; // [sp+38h] [bp-28h]@4
    int v39; // [sp+3Ch] [bp-24h]@9
    u_char *ptr; // [sp+40h] [bp-20h]@9
    char *optval; // [sp+44h] [bp-1Ch]@12

    http_message *a3 = (http_message*)p3;
    //v3 = 0;
    //v31 = a2;
    //v4 = a3;
    if ( a1 != MG_EV_HTTP_REQUEST)//if ( a1 != MG_NEW_REQUEST)
    {
        return;
    }
    v5 = a3->uri.p;

    /**
     * "/http/hls/"
     */
    if ( !strncmp(v5, "/http/hls/", 0xAu) )
    {
        print_wrn(goalbitp2p, v5);
        v6 = (char*)strstr(v5, "hls/");
        needle = v6;
        //v3 = v6;
        if ( !v6 ) {
            return;
        }
        v7 = atoi(v6 + 4);
        //v3 = (void *)v7;
        vSession = v7;
        if ( !v7 ) {
            return;
        }
        asprintf(&needle, "hls/%d", v7);
        v8 = needle;
        v9 = strstr((char*)a3->uri.p, needle);
        v10 = &v9[strlen(v8)];
        free(v8);
        if ( !strncmp(v10, "/manifest", 9u) ) {
            sub_3A70(a2, v7);
            return;
        }
        if ( !strncmp(v10, "/segment", 8u) ) {
            v39 = 0;
            ptr = 0;
            v12 = strstr(v10, "segment/");
            if ( v12 )
            {
                vPiece = atoi(v12 + 8);
                if ( vPiece >= 0 )
                {
                    goalBitManager->GetPieceContent(vSession, vPiece, &ptr, &v39);
                    if ( v39 > 0 )
                    {
                        asprintf(&optval, "video/MP2T");
                        sub_39B8(a2, optval, v39);
                        mg_send(a2, ptr, v39);
                        free(optval);
                        free(ptr);
                        return;
                    }
                } else {
                    return;
                }
            }
            return;//return dword_70C0;
        }
        return;
    }

    /**
     * "/http/progressive/"
     */
    v16 = strncmp(v5, "/http/progressive/", 0x12u);
    //v17 = (void *) v16;
    if (!v16) {
        v18 = strstr(v5, "progressive/");
        if (!v18) {
            return;
        }
        v19 = atoi(v18 + 12);
        v30 = v19;
        if (!v19) {
            return;
        }
        if (goalBitManager->GetSessionType(v19) != HTTP_PROGRESSIVE_DOWNLOAD) {
            return;
        }
        v39 = -1;
        ptr = 0;
        //v20 = mg_get_client_socket(a2);
        //optval = (char *) 65424;
        //setsockopt(v20, 1, 7, &optval, 4u);
        //optval = (char *) 1;
        //setsockopt(v20, 1, 9, &optval, 4u);
        print_msg(__func__, "Send HTTP header for session %d", v30);
        mg_printf(a2,
                  "%s 200 OK\r\nServer: %s\r\nContent-Type: video/MP2T\r\n\r\n",
                  "HTTP/1.1", "webserver/1.0");
        //Mypthread_mutex_lock(14, &unk_70B8);
        //Mypthread_mutex_unlock(14, &unk_70B8);
        v34 = 1;
        while (!bStopGoalbit) {
            goalbit_block_t* ptr1;
            if (!goalBitManager->IsReady(v30))
                return;
            if (v34 &&
                goalBitManager->CanPrebuffer(v30)) {
                ptr1 = goalBitManager->Prebuffer(v30, &v39);
            } else {
                ptr1 = goalBitManager->GetNextPiece(v30, &v39);
                v34 = 0;
            }
            v21 = v39;
            if (v39) {
                print_msg(__func__, "[http_progressive_download] ERROR: %d",
                          v39);
                //v3 = ptr;
                if (!ptr)
                    return;
                goalbit_block_release(ptr1);
                return;
            }
            if (!ptr) {
                //goto LABEL_53;
                sub_3A28(1000, 0);
                //Mypthread_mutex_lock(15, &unk_70B8);
                //Mypthread_mutex_unlock(15, &unk_70B8);
                return;
            }
            v37 = goalbit_block_buffer(ptr1);
            v33 = goalbit_block_size(ptr1);
            if (v33 > 0) {
                while (1) {
                    v22 = v33 - v21;
                    if (v33 - v21 > 65424)
                        v22 = 65424;
                    v36 = v22 + v21;
                    v35 = goalbit_block_duration(ptr1, v21, v22);
                    //v24 = _aeabi_ui2d();
                    //v25 = _aeabi_ddiv(v24);
                    print_msg(
                            __func__,
                            "[http_progressive_download] Serving %.4f secs of content, %u bytes (from %u to %u)",
                            v25,
                            v26,
                            v22,
                            v21,
                            v22 + v21);
                    mg_send(a2, (char*)v37 + v21, v22);//v27 = mg_write(a2, v37 + v21, v22);
                    //if (v27 != v22)
                    //    break;
                    v21 += v22;
                    if (v36 >= v33) {
                        break;//goto LABEL_38;
                    }
                    if (!v34 && v35 > 10000) {
                        sub_3A28(v35 - 10000, v34);
                    }
                }
                print_msg(__func__,
                          "[http_progressive_download] ERROR: could not send all the content of the segment (%d/%d)\n",
                          v27,
                          v33);
                break;
            }
            goalbit_block_release(ptr1);
            if (!v33) {
                sub_3A28(1000, 0);
            }
            //Mypthread_mutex_lock(16, &unk_70B8);
            //Mypthread_mutex_unlock(16, &unk_70B8);
            return;
        }
    }
    /**
     * "crossdomain.xml"
     */
    v28 = strcmp(v5, "crossdomain.xml");
    if (v28) {
        sub_3C60(a2);
        return;
    }
    return;
}

size_t block_Total_Size(block_t *p_block);

block_t* btContent::VideoPlayerPreBuffer(int *pi_status, goalbit_segment_info_t_0 *p_info) {
    //goalbit_t_0 *v3; // edx@1
    //goalbit_structure_t *v4; // eax@2
    //goalbit_structure_t *v5; // eax@6
    size_t i_segment_id; // ebp@6
    block_t *v7; // eax@6
    double_t d_duration_usec; // ST48_8@7
    float f_duration_sec; // ST58_4@7
    //goalbit_structure_t *v11; // edi@7
    //mtime_t v12; // rax@7
    //int v13; // edi@7
    //goalbit_structure_t *v16; // eax@7
    double v17; // ST30_8@7
    size_t v18; // eax@7
    //block_t *result; // eax@8
    bool b_completed; // [sp+6Fh] [bp-1Dh]@6

    //v3 = this->p_this;
    if ( this->p_this && p_this->p_structure->b_btc_alive )
    {
        if ( p_this->p_structure->b_buffering || p_this->p_structure->b_p2p_syncing )
        {
            *pi_status = 4;
            return 0;//result = 0;
        }
        if ( this->b_prebuffer )
        {
            print_err(__func__,
                      "[BTContent] VideoPlayerPreBuffer - prebuffer method can be called only at the beginning of the playback");
            *pi_status = 7;
            return 0;
        }
        this->b_prebuffer = true;

        Mypthread_mutex_lock(17, &p_this->p_structure->exec_lock);
        i_segment_id = p_this->p_structure->exec_info.i_segment_id;
        Mypthread_mutex_unlock(17, &p_this->p_structure->exec_lock);

        v7 = ReadSegmentContent(0, i_segment_id, 0, 0x2F000u, &b_completed);
        if ( v7 )
        {
            d_duration_usec = this->p_mpegts->parseContent(v7);
            f_duration_sec = (long double)d_duration_usec / 1000000.0;

            Mypthread_mutex_lock(18, &this->p_this->p_structure->exec_lock);
            p_this->p_structure->exec_info.f_abd -= f_duration_sec;
            p_this->p_structure->exec_info.i_played_time = mdate() - d_duration_usec;
            p_this->p_structure->exec_info.f_segment_played = f_duration_sec;//*(float *)(*(_DWORD *)v12 + 60) = f_duration_sec;
            p_this->p_structure->exec_info.i_segment_offset = block_Total_Size(v7);//*(_DWORD *)(v13 + 52) = v14;
            p_this->p_structure->exec_info.b_segment_completed = b_completed;
            Mypthread_mutex_unlock(18, &p_this->p_structure->exec_lock);

            *pi_status = 1;
            p_info->i_quality = 0;
            p_info->f_duration = f_duration_sec;
            p_info->b_discontinuity = false;
            v17 = this->p_this->p_structure->exec_info.f_abd;
            v18 = block_Total_Size(v7);
            print_dbg(__func__,
                      "[BTContent] VideoPlayerPreBuffer - Consuming %.2f seconds of content (quality:%u, segmentID:%u, offset:%u, l"
                              "ength:%u), current ABD: %.2f.",
                      f_duration_sec,
                      0,
                      i_segment_id,
                      0,
                      v18,
                      v17);
            return v7;
        }
        print_err(__func__,
                  "[BTContent] VideoPlayerPreBuffer - No content at the beginning, this should not happen.");
        *pi_status = 7;
        return 0;
    }
    *pi_status = 5;
    return 0;
}

template <typename T>
int array_count(array_t<T> *p_array)
{
    return p_array->i_count;
}

template <typename T>
T* array_item_at_index(array_t<T> *p_array, int i_index)
{
    return p_array->pp_elems[i_index];
}

block_t* block_Alloc(size_t i_size)
{
    block_t* v1; // ebx@1
    unsigned char* v2; // eax@2

    v1 = 0;
    if ( i_size )
    {
        v1 = new block_t();
        v2 = (unsigned char*)malloc(i_size);
        v1->i_buffer_size = i_size;
        v1->i_buffer_index = 0;
        v1->p_buffer = v2;
        memset(v2, 0, i_size);
    }
    return v1;
}

void freeKeyframe(metadata_keyframe_t *p_keyframe)
{
    if ( p_keyframe )
        free(p_keyframe);
}

template <typename T>
void array_clear(array_t<T> *p_array)
{
    free(p_array->pp_elems);
    p_array->i_count = 0;
    p_array->pp_elems = 0;
}

template <typename T>
void array_destroy(array_t<T> *p_array)
{
    if ( p_array )
    {
        array_clear(p_array);
        free(p_array);
    }
}

void freeKeyframesArray(array_t<metadata_keyframe_t> *p_keyframes)
{
    int v1; // ebx@1
    metadata_keyframe_t *v2; // eax@3

    v1 = 0;
    if ( p_keyframes )
    {
        while ( v1 < array_count(p_keyframes) )
        {
            v2 = array_item_at_index<metadata_keyframe_t>(p_keyframes, v1);
            if ( v2 )
                freeKeyframe(v2);
            ++v1;
        }
        array_destroy(p_keyframes);
    }
}

void freePiece(metadata_piece_t *p_piece)
{
    if ( p_piece )
    {
        if ( p_piece->psz_MD5 )
            free(p_piece->psz_MD5);
        free(p_piece);
    }
}
void freePiecesArray(array_t<metadata_piece_t> *p_pieces)
{
    int v1; // ebx@1
    metadata_piece_t *v2; // eax@3

    v1 = 0;
    if ( p_pieces )
    {
        while ( v1 < array_count(p_pieces) )
        {
            v2 = array_item_at_index<metadata_piece_t>(p_pieces, v1);
            if ( v2 )
                freePiece(v2);
            ++v1;
        }
        array_destroy(p_pieces);
    }
}

block_t* btContent::ReadSegmentContent(size_t i_quality, size_t i_segment_id, size_t i_start_offset,
                                       size_t i_length, bool *pb_completed) {
    array_t<metadata_keyframe_t> *v6; // ebp@1
    //goalbit_structure_t *v8; // eax@1
    int v10; // ebx@7
    block_t *v13; // esi@8
    unsigned char* v16; // ST28_4@8
    unsigned char* v17; // ST30_4@8
    metadata_piece_t *v19; // esi@11
    //block_t *v20; // eax@15
    char v21; // bl@15
    int v22; // ebx@21
    size_t v24; // edi@22
    metadata_keyframe_t *v25; // esi@22
    block_t *v26; // esi@30
    block_t *v28; // eax@48
    size_t v29; // eax@51
    size_t v30; // eax@52
    size_t i_end_offset; // [sp+30h] [bp-3Ch]@21
    array_t<metadata_piece_t> *p_pieces; // [sp+34h] [bp-38h]@1
    bool v33; // [sp+3Bh] [bp-31h]@20
    block_t *p_full_content; // [sp+48h] [bp-24h]@1
    block_t *p_piece_content; // [sp+4Ch] [bp-20h]@8

    v6 = 0;
    *pb_completed = false;
    p_full_content = 0;
    print_dbg(__func__,
              "[BTContent] ReadSegmentContent - quality:%u, segment:%u, start:%u, length:%u",
              i_quality,
              i_segment_id,
              i_start_offset,
              i_length);
    p_pieces = 0;
    //v8 = this->p_this->p_structure;
    if ( p_this->p_structure->p_metadataManager && p_this->p_structure->b_p2p_enabled ) {
        v6 = p_this->p_structure->p_metadataManager->getSegmentKeyframes(i_quality, i_segment_id);
        p_pieces = p_this->p_structure->p_metadataManager->getSegmentPieces(i_quality, i_segment_id);
    }
    if ( this->p_hls_bitfield && this->p_hls_bitfield->IsSet(i_quality, i_segment_id) ) {
        print_dbg(__func__, "[BTContent] ReadSegmentContent - We have the complete segement");
        v21 = 1;
        p_full_content = ReadSegment(i_quality, i_segment_id);
    } else {
        if ( p_pieces ) {
            v10 = 0;
            print_dbg(__func__, "[BTContent] ReadSegmentContent - Filling the content with pieces");
            while ( v10 < array_count(p_pieces) ) {
                v19 = array_item_at_index(p_pieces, v10);
                if ( !p_p2p_bitfield->IsSet(i_quality, v19->i_piece_id ) )
                    break;
                p_piece_content = ReadPiece(i_quality, v19->i_piece_id );
                if ( !p_piece_content )
                    break;
                print_dbg(__func__, "[BTContent] ReadSegmentContent - Adding the content of piece %u", v19->i_piece_id );
                if (p_full_content == 0) {
                    p_full_content = p_piece_content;
                } else {
                    v13 = block_Alloc(block_Total_Size(p_full_content) + block_Total_Size(p_piece_content));
                    memcpy(v13->p_buffer, p_full_content->p_buffer, block_Total_Size(p_full_content));
                    v16 = p_piece_content->p_buffer;
                    v17 = v13->p_buffer;
                    memcpy(v17 + block_Total_Size(p_full_content), v16, block_Total_Size(p_piece_content));
                    block_Release(p_piece_content);
                    block_Release(p_full_content);
                    p_full_content = v13;
                }
                ++v10;
            }
        }
        v21 = 0;
    }
    if ( p_full_content )
    {
        if (i_start_offset < block_Total_Size(p_full_content)) {
            if (!v21) {
                if (!v6 && !i_length) {
                    v26 = 0;
                    print_dbg(__func__,
                              "[BTContent] ReadSegmentContent - There is no keyframe info");
                    goto ReadSegmentContent_return;
                }
                goto LABEL_20;
            }
            if (i_length) {
                LABEL_20:
                v33 = i_length != 0;
                if (v6) {
                    v22 = 0;
                    i_end_offset = 0;
                    while (v22 < array_count(v6)) {
                        v25 = array_item_at_index(v6, v22);
                        v24 = v25->i_offset;
                        if (v24 > block_Total_Size(p_full_content))
                            break;
                        if (i_start_offset < v25->i_offset) {
                            if (v33 && i_length < v25->i_offset) {
                                i_end_offset = v25->i_offset;
                                goto LABEL_29;
                            }
                            i_end_offset = v25->i_offset;
                        }
                        ++v22;
                    }
                    if (!i_end_offset)
                        goto LABEL_43;
                } else {
                    i_end_offset = 0;
                    LABEL_43:
                    if (v33) {
                        i_end_offset = i_length;
                        if (i_length >= block_Total_Size(p_full_content)) {
                            i_end_offset = block_Total_Size(p_full_content);
                            print_wrn(__func__,
                                      "[BTContent] ReadSegmentContent - chopping content on non-keyframe offset (%u bytes)",
                                      i_end_offset);
                        } else {
                            print_wrn(__func__,
                                      "[BTContent] ReadSegmentContent - chopping content on non-keyframe offset (%u bytes)",
                                      i_length);
                        }
                    }
                }
                //goto LABEL_29;
            } else {
                v28 = p_full_content;
                *pb_completed = true;
                i_end_offset = block_Total_Size(v28);
            }
            LABEL_29:
            if (i_end_offset) {
                v26 = block_Alloc(i_end_offset - i_start_offset);
                memcpy(v26->p_buffer, &p_full_content->p_buffer[i_start_offset],
                       i_end_offset - i_start_offset);
                v26->i_buffer_index = i_end_offset - i_start_offset;
                print_dbg(__func__,
                          "[BTContent] ReadSegmentContent - Returning content from %u to %u, %d bytes",
                          i_start_offset,
                          i_end_offset,
                          i_end_offset - i_start_offset);
            } else {
                v26 = 0;
                v29 = block_Total_Size(p_full_content);
                print_dbg(__func__,
                          "[BTContent] ReadSegmentContent - There is not enough content (%d bytes), end offset == 0",
                          v29);
            }
        } else {
            v26 = 0;
            v30 = block_Total_Size(p_full_content);
            print_dbg(__func__,
                      "[BTContent] ReadSegmentContent - There is not enough content (%d bytes), start offset < full content.",
                      v30);
            //goto ReadSegmentContent_return;
        }
    } else {
        v26 = 0;
        print_dbg(__func__, "[BTContent] ReadSegmentContent - There is no content.");
    }
    ReadSegmentContent_return:
    if ( v6 )
        freeKeyframesArray(v6);
    if ( p_pieces )
        freePiecesArray(p_pieces);
    if ( p_full_content )
        block_Release(p_full_content);
    return v26;
}

/*
 void ReportEvent(goalbit_t_0 *p_this, goalbit_event_t_0 *p_event)
 {
 goalbit_event_t_0 *v2; // ebx@1
 goalbit_param_t_0 *v3; // ecx@1
 void *(*v4)(goalbit_t_0 *, goalbit_event_t_0 *); // edx@1

 v2 = p_event;
 v3 = p_this->p_param;
 v4 = v3->pf_event_callback;
 if ( v4 )
 {
 v2->p_ref = v3->p_external_ref;
 v4(p_this, v2);//goalbit_event_callback
 }
 free(v2);
 }
 */

block_t *btContent::VideoPlayerRead(int *pi_status, goalbit_segment_info_t_0 *p_info) {
    //goalbit_structure_t *v3; // eax@2
    //mtime_t v4; // rax@5
    //goalbit_structure_t *v5; // ecx@5
    //goalbit_structure_t *v6; // eax@6
    int64_t v7; // ebx@6
    //int v8; // esi@6
    double_t v9; // esi@10
    //goalbit_structure_t *v10; // ebx@10
    //goalbit_structure_t *v11; // ebx@12
    //goalbit_t_0 *v16; // eax@12
    //goalbit_structure_t *v17; // eax@12
    //goalbit_t_0 *v19; // eax@13
    //goalbit_structure_t *v20; // eax@14
    block_t *result; // eax@14
    //goalbit_structure_t *v22; // ebx@19
    //mtime_t v23; // rax@19
    size_t v24; // ebx@20
    //signed int v28; // eax@24
    //u_char *v29; // edi@24
    //u_char *v30; // esi@24
    //signed int v35; // eax@32
    //u_short *v36; // edi@32
    //u_char *v37; // esi@32
    //mtime_t v39; // rax@37
    //u_char v40; // cf@37
    //goalbit_structure_t *v41; // ebx@8
    //mtime_t v42; // rax@8
    long double v43; // fst7@39
    //goalbit_t_0 *v44; // eax@39
    //goalbit_structure_t *v45; // ebx@40
    //mtime_t v46; // rax@40
    goalbit_event_t_0 *v47; // eax@41
    goalbit_event_t_0 *v48; // eax@41
    //goalbit_structure_t *v49; // ebx@41
    //mtime_t v50; // rax@41
    //int16_t v51; // ax@31
    //u_char v52; // dl@29
    //int16_t v53; // ax@23
    //u_char v54; // al@21
    block_t *p_block; // [sp+30h] [bp-4Ch]@9
    size_t i_segment_offset; // [sp+34h] [bp-48h]@6
    size_t i_exec_index; // [sp+38h] [bp-44h]@6
    float f_duration_sec; // [sp+3Ch] [bp-40h]@10
    float f_abd; // [sp+48h] [bp-34h]@6
    block_t *p_raw_block; // [sp+58h] [bp-24h]@20
    bool b_completed; // [sp+5Fh] [bp-1Dh]@6

    if ( this->p_this && (p_this->p_structure->b_btc_alive) )
    {
        if ( p_this->p_structure->b_buffering || p_this->p_structure->b_p2p_syncing )
        {
            *pi_status = 4;
            return 0;
        }
        //v4 = mdate();
        //v5 = this->p_this->p_structure;
        if ( mdate() > p_this->p_structure->i_last_sync_date) {
            this->b_prebuffer = true;

            Mypthread_mutex_lock(19, &p_this->p_structure->exec_lock);
            v7 = p_this->p_structure->exec_info.i_played_time;
            f_abd = p_this->p_structure->exec_info.f_abd;
            i_exec_index = p_this->p_structure->exec_info.i_segment_id;
            //v8 = HIDWORD(v6->exec_info.i_played_time);
            i_segment_offset = p_this->p_structure->exec_info.i_segment_offset;
            b_completed = p_this->p_structure->exec_info.b_segment_completed;
            Mypthread_mutex_unlock(19, &p_this->p_structure->exec_lock);

            if ( b_completed )
            {
                ++i_exec_index;
                Mypthread_mutex_lock(20, &this->p_this->p_structure->exec_lock);
                p_this->p_structure->exec_info.i_segment_offset = 0;
                p_this->p_structure->exec_info.i_segment_id = i_exec_index;
                p_this->p_structure->exec_info.b_segment_completed = false;
                p_this->p_structure->exec_info.i_played_time = mdate();
                this->p_this->p_structure->exec_info.f_segment_played = 0;
                Mypthread_mutex_unlock(20, &this->p_this->p_structure->exec_lock);
                i_segment_offset = 0;
            } else if ( !v7 ) {
                Mypthread_mutex_lock(21, &this->p_this->p_structure->exec_lock);
                p_this->p_structure->exec_info.i_played_time = mdate();
                this->p_this->p_structure->exec_info.f_segment_played = 0;
                Mypthread_mutex_unlock(21, &this->p_this->p_structure->exec_lock);
            }
            print_dbg(__func__, "[BTContent] VideoPlayerRead - trying to read content");
            p_block = ReadSegmentContent(0, i_exec_index, i_segment_offset, 0, &b_completed);
            if ( p_block )
            {
                print_dbg(__func__, "[BTContent] VideoPlayerRead - processing MPEGTS content");
                v9 = this->p_mpegts->parseContent(p_block);
                f_duration_sec = v9 / 1000000.0;
                if ( p_this->p_structure->i_last_sync_date )
                {
                    p_this->p_structure->i_last_sync_date += v9;
                }
                else
                {
                    p_this->p_structure->i_last_sync_date = mdate() + v9;
                }

                Mypthread_mutex_lock(22, &p_this->p_structure->exec_lock);
                p_this->p_structure->exec_info.i_quality = 0;
                p_this->p_structure->exec_info.i_segment_offset = p_this->p_structure->exec_info.i_segment_offset+block_Total_Size(p_block);
                p_this->p_structure->exec_info.b_segment_completed = b_completed;
                Mypthread_mutex_unlock(22, &p_this->p_structure->exec_lock);

                if ( i_segment_offset )
                {
                    print_dbg(__func__, "[BTContent] VideoPlayerRead - adding the PAT/PMT tables");
                    p_raw_block = p_block;
                    v24 = block_Total_Size(p_block) + 0x178;
                    p_block = block_Alloc(v24);
                    memcpy(p_block->p_buffer, this->p_mpegts->p_pat, 0x0BC);
                    memcpy(p_block->p_buffer+0xBC, this->p_mpegts->p_pmt, 0xBC);
                    memcpy(p_block->p_buffer+0x178, p_raw_block->p_buffer, block_Total_Size(p_raw_block));
                    p_block->i_buffer_index = v24;
                    block_Release(p_raw_block);
                }
                *pi_status = 1;
                p_info->f_duration = f_duration_sec;
                p_info->i_quality = 0;
                p_info->b_discontinuity = 0;
                print_dbg(__func__,
                          "[BTContent] VideoPlayerRead - Consuming %.4f seconds of content (quality:%u, segmentID:%u, offset:%u, length"
                                  ":%u), current ABD: %.2f.",
                          f_duration_sec,
                          0,
                          i_exec_index,
                          i_segment_offset,
                          block_Total_Size(p_block),
                          f_abd);
                //v19 = this->p_this;
            }
            else
            {
                //--------------------------------------debug
                //p_block = ReadSegmentContent(0, i_exec_index, i_segment_offset, 0, &b_completed);
                print_err(__func__,
                          "[BTContent] VideoPlayerRead - Couldn't consume content (quality:%u, segmentID:%u, offset:%u), current ABD: %.2f",
                          0,
                          i_exec_index,
                          i_segment_offset,
                          f_abd);
                *pi_status = 2;
                p_info->i_quality = 0x80000001;
                p_info->b_discontinuity = 1;
                v43 = this->p_this->p_structure->p_hlsManager->getSegmentDuration(0, i_exec_index);
                //v44 = this->p_this;
                p_info->f_duration = v43;
                if ( v43 == 0.0 )
                    p_info->f_duration = p_this->p_structure->f_avg_usec_x_segment / 1000000.0;
                Mypthread_mutex_lock(23, &p_this->p_structure->exec_lock);
                //v45 = this->p_this->p_structure;
                p_this->p_structure->b_buffering = true;
                p_this->p_structure->exec_info.i_segment_offset = 0;
                p_this->p_structure->exec_info.b_segment_completed = 0;
                //LODWORD(v45->exec_info.i_played_time) = 0;
                //HIDWORD(v45->exec_info.i_played_time) = 0;
                p_this->p_structure->exec_info.i_played_time = 0;
                //LODWORD(v45->exec_info.f_segment_played) = 0;
                p_this->p_structure->exec_info.f_segment_played = 0;
                //v46 = mdate();
                //LODWORD(v45->i_last_buff_start) = v46;
                //LODWORD(v46) = this->p_this;
                //HIDWORD(v45->i_last_buff_start) = HIDWORD(v46);
                //LODWORD(v46) = *(_DWORD *)v46;
                p_this->p_structure->i_last_buff_start = mdate();
                //*(_DWORD *)(v46 + 0xA4) = 0;
                //*(_DWORD *)(v46 + 0xA8) = 0;
                this->p_this->p_structure->i_last_sync_date = 0;
                this->b_prebuffer = false;
                Mypthread_mutex_unlock(23, &p_this->p_structure->exec_lock);
                SyncWindows();
                //v19 = this->p_this;
                /*
                 if ( this->p_this->p_param->pf_event_callback )
                 {
                 v47 = new goalbit_event_t_0();
                 v47->i_event = 1;
                 v47->g_value.i_int = 0;
                 ReportEvent(this->p_this, v47);//goalbit_event_callback
                 v48 = new goalbit_event_t_0();
                 v48->i_event = 2;
                 v48->g_value.i_int = 0;
                 ReportEvent(this->p_this, v48);//goalbit_event_callback
                 //v49 = this->p_this->p_structure;
                 //v50 = mdate();
                 //LODWORD(v49->i_last_exec_report) = v50;
                 p_this->p_structure->i_last_exec_report = mdate();
                 //v19 = this->p_this;
                 //HIDWORD(v49->i_last_exec_report) = HIDWORD(v50);
                 }
                 */
                this->p_this->p_param->p_external_ref->SetBufferingState(true);
                this->p_this->p_param->p_external_ref->SetBufferingPercentage(0);
                p_this->p_structure->i_last_exec_report = mdate();

            }
            Mypthread_mutex_lock(24, &p_this->p_structure->exec_lock);
            p_this->p_structure->b_check_win = true;
            Mypthread_mutex_unlock(24, &p_this->p_structure->exec_lock);
            result = p_block;
        }
        else
        {
            result = 0;
            *pi_status = 3;
        }
    }
    else
    {
        *pi_status = 5;
        result = 0;
    }
    return result;
}

btContent::btContent(goalbit_t_0 *p_goalbit) {
    this->p_this = p_goalbit;
    //v2 = (btCache *)operator new(0x2Cu);
    //btCache::btCache(v2, p_goalbit);
    this->p_cache = new btCache(p_goalbit);
    //v3 = (MpegTS *)operator new(0x10u);
    //MpegTS::MpegTS(v3, p_goalbit);
    this->p_mpegts = new MpegTS(p_goalbit);
    this->p_p2p_bitfield = 0;
    this->p_hls_bitfield = 0;
    this->p_hls_discontinuity = 0;
    this->p_hls_error = 0;
    time(&this->m_start_timestamp);
    this->fd_status = 0;
    this->i_last_abi = 0LL;
    this->i_last_abi_dup = 0LL;
    this->i_last_recv = (this->p_this->p_structure->f_hls_buffer_size
                         * this->p_this->p_structure->f_avg_usec_x_segment)
                        + mdate();
    this->i_last_quality_down = 0;
    this->i_same_quality_count = 0;
    this->b_prebuffer = false;
    this->i_last_opt_download = mdate() + 8000000;
    return;
}

/**
 * setting btContent.exec_info
 * @param p_goalbit
 * @param i_hls_offset
 */
void define_HLSWindowOffset(goalbit_t_0 *p_goalbit, size_t i_hls_offset)
{
    //size_t v2; // edi@1
    //size_t v3; // esi@1
    //goalbit_t_0 *i; // ebx@1
    //goalbit_structure_t *v5; // eax@3
    //size_t v6; // edx@3
    //size_t v7; // edx@5

    //size_t v2 = i_hls_offset;
    Mypthread_mutex_lock(25, &p_goalbit->p_structure->exec_lock);
    //v5 = i->p_structure;
    //v6 = p_goalbit->p_structure->exec_info.i_segment_id;
    if ( p_goalbit->p_structure->exec_info.i_segment_id == 0x80000001 || i_hls_offset > p_goalbit->p_structure->exec_info.i_segment_id )
        p_goalbit->p_structure->exec_info.i_segment_id = i_hls_offset;
    //v7 = p_goalbit->p_structure->exec_info.i_abi;
    if ( p_goalbit->p_structure->exec_info.i_abi == 0x80000001 || i_hls_offset > p_goalbit->p_structure->exec_info.i_abi )
        p_goalbit->p_structure->exec_info.i_abi = i_hls_offset;
    size_t v3 = i_hls_offset;
    for ( ; v3 & 7; --v3 )
        ;
    p_goalbit->p_structure->i_hls_win_offset = v3;
    print_dbg(__func__,
              "New HLS window [%d, %d] - exec=%d, abi=%u, abd=%f",
              v3,
              v3 + p_goalbit->p_structure->i_hls_win_length,
              p_goalbit->p_structure->exec_info.i_segment_id,
              p_goalbit->p_structure->exec_info.i_abi,
              p_goalbit->p_structure->exec_info.f_abd);
    Mypthread_mutex_unlock(25, &p_goalbit->p_structure->exec_lock);
    return;
}

void define_P2PWindowOffset(goalbit_t_0 *p_goalbit, size_t i_p2p_offset)
{
    size_t v3 = i_p2p_offset + 1;
    if ( (i_p2p_offset + 1) & 7 )
    {
        while ( i_p2p_offset & 7 )
            --i_p2p_offset;
        v3 = i_p2p_offset;
    }
    Mypthread_mutex_lock(26, &p_goalbit->p_structure->exec_lock);
    //v4 = v2->p_structure;
    p_goalbit->p_structure->i_p2p_win_offset = v3;
    Mypthread_mutex_unlock(26, &p_goalbit->p_structure->exec_lock);
    print_dbg(goalbitp2p,
              "New P2P window [%d, %d]",
              p_goalbit->p_structure->i_p2p_win_offset,
              p_goalbit->p_structure->i_p2p_win_offset + p_goalbit->p_structure->i_p2p_win_length);
}

void btContent::WindowCheck() {
    //goalbit_structure_t *v1; // eax@1
    size_t v2; // esi@1
    int v3; // ebp@1
    float v6; // ST20_4@1
    unsigned int v7; // ecx@2
    size_t v8; // edi@4
    char v9; // si@4
    //goalbit_structure_t *v10; // eax@4
    //goalbit_structure_t *v11; // eax@6
    size_t v12; // esi@8
    size_t v13; // edi@8
    int i; // ST04_4@10
    //unsigned int v15; // eax@11
    //bool v16; // dl@11
    //goalbit_structure_t *v17; // eax@15
    int v18; // eax@17
    size_t v19; // ecx@18
    float v21; // ST20_4@18
    unsigned int v22; // ecx@19
    size_t v23; // esi@21
    size_t v24; // eax@26
    //goalbit_t_0 *v25; // eax@30
    int i_p2p_abi; // [sp+Ch] [bp-60h]@2
    size_t i_old_p2p_offset; // [sp+38h] [bp-34h]@1
    size_t i_exec_index; // [sp+3Ch] [bp-30h]@1

    Mypthread_mutex_lock(27, &this->p_this->p_structure->exec_lock);
    v2 = p_this->p_structure->i_hls_win_offset;
    v3 = p_this->p_structure->exec_info.i_abi;
    i_exec_index = p_this->p_structure->exec_info.i_segment_id;
    i_old_p2p_offset = p_this->p_structure->i_p2p_win_offset;
    Mypthread_mutex_unlock(27, &p_this->p_structure->exec_lock);

    v6 = (long double)(0x64 * (v3 - v2)) / (long double)this->p_this->p_structure->i_hls_win_length;
    print_dbg(__func__, "[BTContent] WindowCheck - Checking HLS window offset: [%u--|%u|-->%u]: %0.2f %c",
              v2,
              v3,
              v2 + this->p_this->p_structure->i_hls_win_length,
              v6,
              '%');
    if ( v6 <= 80.0 )
    {
        v9 = 1;
    }
    else
    {
        i_p2p_abi = v2;
        v7 = (signed int)((long double)this->p_this->p_structure->i_hls_win_length * 0.3);
        if ( v7 <= 7 )
            v7 = 8;
        v8 = v7 + v2;
        v9 = 1;
        print_dbg(__func__,
                  "[BTContent] WindowCheck - trying to set the new HLS window offset as: %u [old HLS offset (%u) + %u]",
                  v8,
                  i_p2p_abi,
                  v7);
        define_HLSWindowOffset(this->p_this, v8);
        UpdateHLSWindow(this->p_this->p_structure->i_hls_win_offset);
        //v10 = this->p_this->p_structure;
        if ( p_this->p_structure->b_p2p_enabled )
        {
            v24 = p_this->p_structure->p_metadataManager->segmentID2PieceID(p_this->p_structure->i_hls_win_offset, 0x80000001, 2, 1);
            if ( v24 == 0x80000001 )
            {
                //v25 = this->p_this;
                p_this->p_structure->b_p2p_enabled = false;
                print_err(__func__, "[BtContent] WindowCheck - P2P DISABLED!!");
            }
            else if ( i_old_p2p_offset < v24 )
            {
                v9 = 0;
                define_P2PWindowOffset(this->p_this, v24);
                if ( this->p_this->p_structure->i_p2p_win_offset != i_old_p2p_offset )
                    UpdateP2PWindow(this->p_this->p_structure->i_p2p_win_offset);
            }
        }
        this->p_cache->UpdateWindow();
    }
    if ( p_this->p_structure->b_p2p_enabled )
    {
        if ( v9 ) {
            v18 = p_this->p_structure->p_metadataManager->segmentID2PieceID(v3, 0x80000001, 4, 2);
            if (v18 != 0x80000001) {
                v19 = this->p_this->p_structure->i_p2p_win_length;
                v21 = (long double) (0x64 * (v18 - i_old_p2p_offset)) / (long double) v19;
                //            print_dbg(__func__,                    "[BTContent] WindowCheck - Checking P2P window offset: [%u--|%u|-->%u]: %0.2f %c",
                //                    i_old_p2p_offset,
                //                    v18,
                //                    i_old_p2p_offset + v19,
                //                    (double)v20,
                //                    0x25);
                if (v21 > 80.0) {
                    v22 = ((long double) this->p_this->p_structure->i_p2p_win_length * 0.3);
                    if (v22 <= 7)
                        v22 = 8;
                    v23 = v22 + i_old_p2p_offset;
                    print_dbg(__func__,
                              "[BTContent] WindowCheck - trying to set the new P2P window offset as: %u [old P2P offset (%u) + %u]",
                              v22 + i_old_p2p_offset,
                              i_old_p2p_offset,
                              v22);
                    define_P2PWindowOffset(this->p_this, v23);
                    if (this->p_this->p_structure->i_p2p_win_offset != i_old_p2p_offset)
                        UpdateP2PWindow(this->p_this->p_structure->i_p2p_win_offset);
                }
                this->p_cache->UpdateWindow();
            }
        }
        v12 = i_old_p2p_offset;
        v13 = p_this->p_structure->p_metadataManager->segmentID2PieceID(i_exec_index, 0x80000001, 3, 1);
        if ( v13 != 0x80000001 )
        {
            while ( v12 < v13 )
            {
                this->p_this->p_structure->WORLD->DeleteExpiredRequest(v12);
                v12++;
            }
        }
    }
    Mypthread_mutex_lock(28, &p_this->p_structure->exec_lock);
    p_this->p_structure->b_check_win = false;
    Mypthread_mutex_unlock(28, &p_this->p_structure->exec_lock);
}

void btContent::UpdateHLSWindow(size_t i_new_hls_offset) {
    this->p_hls_bitfield->UpdateWindow(i_new_hls_offset);
    this->p_hls_discontinuity->UpdateWindow(i_new_hls_offset);
    this->p_hls_error->UpdateWindow(i_new_hls_offset);
}

void btContent::BitFieldP2PUpdate() {

    size_t idx; // ST08_4@10
    int v5; // ebx@10
    array_t<metadata_piece_t> *v6; // edi@10
    metadata_piece_t *v8; // esi@12

    if ( p_this->p_structure->b_p2p_enabled == false ) {
        return;
    }
    if ( this->p_hls_bitfield  == 0 ) {
        if ( this->p_p2p_bitfield )
        {
            delete this->p_p2p_bitfield;
        }
        this->p_p2p_bitfield = 0;
    }
    if ( !this->p_p2p_bitfield )
    {
        InitP2PBitfield();
    }
    size_t i_quality = 0;
    if ( p_this->p_structure->i_hls_qualities )
    {
        do
        {
            int v2 = 0;
            if ( p_this->p_structure->i_hls_win_length )
            {
                do
                {
                    if ( this->p_hls_bitfield->IsPieceSet(i_quality, v2 + p_this->p_structure->i_hls_win_offset) )
                    {
                        idx = p_this->p_structure->i_hls_win_offset + v2;
                        v5 = 0;
                        v6 = p_this->p_structure->p_metadataManager->getSegmentPieces(i_quality, idx);
                        if ( v6 )
                        {
                            while ( v5 < array_count(v6) )
                            {
                                v8 = array_item_at_index(v6, v5);
                                if ( v8 )
                                {
                                    if ( !this->p_p2p_bitfield->IsSet(i_quality, v8->i_piece_id) )
                                    {
                                        this->p_p2p_bitfield->Set(i_quality, v8->i_piece_id);
                                        this->p_this->p_structure->WORLD->Tell_World_I_Have(i_quality, v8->i_piece_id);
                                    }
                                    freePiece(v8);
                                }
                                ++v5;
                            }
                            array_destroy(v6);
                        }
                    }
                }
                while ( ++v2 < p_this->p_structure->i_hls_win_length );
            }
        }
        while ( ++i_quality < p_this->p_structure->i_hls_qualities );
    }
    return;
}

unsigned int btContent::InitP2PBitfield() {

    this->p_p2p_bitfield = (btSelfBitField *)new btBitField(this->p_this, this->p_this->p_structure->i_hls_qualities, 0);
    if ( this->p_p2p_bitfield )
    {
        this->p_cache->UpdateWindow();
        return 0;
    }
    return -1;
}

void btContent::InitHLSExecution() {
    //goalbit_structure_t *v1; // esi@1
    //bool v2; // al@1
    //goalbit_t_0 *v3; // ecx@1
    //goalbit_structure_t *v4; // edx@1
    //goalbit_t_0 *v6; // eax@1
    //goalbit_qualities_desc_t_0 *v7; // esi@2
    //goalbit_event_t_0 *v8; // eax@2
    //goalbit_event_t_0 *v10; // edx@2
    //goalbit_t_0 *v11; // eax@2

    /**
     * set goalbit_structure_t b_hls_live
     */
    //v1 = this->p_this->p_structure;
    //v2 = p_this->p_structure->p_hlsManager->isLiveContent();
    //v3 = this->p_this;
    //v4 = this->p_this->p_structure;
    p_this->p_structure->b_hls_live = p_this->p_structure->p_hlsManager->isLiveContent();

    p_this->p_structure->i_target_quality = p_this->p_structure->i_hls_qualities - 1;
    p_this->p_structure->exec_info.i_quality = p_this->p_structure->i_target_quality;
    print_dbg(__func__, "[BTCONTENT] Initial quality selected: %d", p_this->p_structure->i_target_quality);
    //v6 = this->p_this;
    //    if ( this->p_this->p_param->pf_event_callback ) {
    //        /**
    //         * goalbitsession <- goalbit_qualities_desc_t_01
    //         */
    //        //v7 = p_this->p_structure->p_hlsManager->exportStreamsDescription();
    //        v8 = new goalbit_event_t_0();
    //        v8->i_event = 3;
    //        v8->g_value.goalbit_qualities_desc_t_01 = p_this->p_structure->p_hlsManager->exportStreamsDescription();
    //        ReportEvent(this->p_this, v8);//goalbit_event_callback
    //
    //        /**
    //         *
    //         */
    //        v10 = new goalbit_event_t_0();
    //        v10->i_event = 4;
    //        //v11 = this->p_this;
    //        v10->g_value.i_int = this->p_this->p_structure->i_target_quality;
    //        ReportEvent(p_this, v10);//goalbit_event_callback
    //        //v6 = this->p_this;
    //    }

    this->p_this->p_param->p_external_ref->SetQualitiesDesc(p_this->p_structure->p_hlsManager->exportStreamsDescription());
    size_t v12 = p_this->p_structure->p_hlsManager->getInitialSequence();
    print_msg(__func__,
              "[BTCONTENT] start dowloading segments [quality %d ---> segment %d]",
              this->p_this->p_structure->i_target_quality,
              v12);
    define_HLSWindowOffset(this->p_this, v12);
    InitHLSBitfield();
    InitP2PInfo();
}

/**
 * setting btContent.p_hls_bitfield, p_hls_discontinuity, p_hls_error
 * @return
 */
int btContent::InitHLSBitfield() {
    //btBitField *v1; // esi@1
    //btBitField *v2; // esi@2
    //btBitField *v3; // esi@3

    //v1 = new btBitField(this->p_this, this->p_this->p_structure->i_hls_qualities, true);
    this->p_hls_bitfield = (btSelfBitField *)new btBitField(this->p_this, this->p_this->p_structure->i_hls_qualities,
                                                            true);
    if ( this->p_hls_bitfield
         && (this->p_hls_discontinuity = (btSelfBitField *)new btBitField(this->p_this, this->p_this->p_structure->i_hls_qualities, true),
                //btBitField::btBitField(v2, this->p_this, this->p_this->p_structure->i_hls_qualities, 1),
                (this->p_hls_discontinuity != 0)
                && (this->p_hls_error = (btSelfBitField *)new btBitField(this->p_this, this->p_this->p_structure->i_hls_qualities, true),
            //btBitField::btBitField(v3, this->p_this, this->p_this->p_structure->i_hls_qualities, 1),
            this->p_hls_error != 0 )))
    {
        this->p_cache->UpdateWindow();
        return 0;
    }
    return -1;
}

int btContent::InitP2PInfo() {
    //goalbit_t_0 *v1; // eax@1
    //char *v2; // ebx@1
    uint32_t v3; // ebp@1
    unsigned __int8 *data; // ST04_4@1
    char *v5; // ebx@1
    char *v6; // esi@1
    char *v7; // edx@1
    char i; // al@1
    char v9; // al@4
    int result; // eax@6
    //int v11; // edx@6
    SHA1_CTX v12; // [sp+10h] [bp-7Ch]@1
    //int v13; // [sp+6Ch] [bp-20h]@1

    //v13 = *MK_FP(__GS__, 0x14);
    //v1 = this->p_this;
    memset(this->m_shake_buffer, 0, sizeof(this->m_shake_buffer));
    this->m_shake_buffer[0] = 0x10;
    strcpy((char*)&this->m_shake_buffer[1], "GoalBitStreaming");
    v3 = strlen(p_this->p_param->psz_content_id);
    SHA1Init(&v12);
    data = (unsigned __int8*)p_this->p_param->psz_content_id;
    v5 = (char *)&this->m_shake_buffer[0x2D];
    SHA1Update(&v12, data, v3);
    SHA1Final(&this->m_shake_buffer[0x19], &v12);
    v6 = (char *)&this->m_shake_buffer[0x41];
    v7 = this->p_this->p_structure->arg_user_agent;
    for ( i = *v7; *v7; ++v5 )
    {
        ++v7;
        *v5 = i;
        i = *v7;
    }
    for ( ; v6 > v5; ++v5 )
    {
        do
            v9 = random();
        while ( !v9 );
        *v5 = v9;
    }
    result = 0;
    //v11 = *MK_FP(__GS__, 0x14) ^ v13;
    return result;
}

void btContent::InitP2PExecution() {

    size_t v1 = p_this->p_structure->p_metadataManager->segmentID2PieceID(
            p_this->p_structure->i_hls_win_offset,
            0x80000001,
            2,
            1);
    if ( v1 != 0x80000001 )
    {
        define_P2PWindowOffset(this->p_this, v1);
        p_this->p_structure->b_p2p_enabled = true;
        BitFieldP2PUpdate();
        //v2 = this->p_this->p_structure;
        print_wrn(goalbitp2p,
                  "[DOWNLOADER] P2P ENABLED!! HLS_window[%d,%d], P2P_window[%d,%d]",
                  p_this->p_structure->i_hls_win_offset,
                  p_this->p_structure->i_hls_win_offset + p_this->p_structure->i_hls_win_length,
                  p_this->p_structure->i_p2p_win_offset,
                  p_this->p_structure->i_p2p_win_offset + p_this->p_structure->i_p2p_win_length);
    }
}

void btContent::ProcessDownloadedHLSSegments() {

    while ( this->p_this->p_structure->p_hlsManager->hasSegmentsReady() ) {
        quality_segmentid *v1 = this->p_this->p_structure->p_hlsManager->getNextSegmentReady();
        if ( !v1 )
            break;
        size_t v2 = v1->i_quality;
        size_t v3 = v1->i_segment_id;
        free(v1);
        print_dbg(__func__, "[btContent] ProcessDownloadedHLSSegments - segment %d ready for quality %d", v3, v2);
        hls_segment_t *v5 = this->p_this->p_structure->p_hlsManager->getSegment(v2, v3);
        if ( v5 )
        {
            block_t *v6 = v5->p_data;
            if ( v6 )
            {
                WriteSegment(v2, v3, v6);
                if ( v5->b_discontinuity )
                {
                    btSelfBitField *v7 = this->p_hls_discontinuity;
                    if ( v7 )
                        v7->SetPiece(v2, v3);
                }
                this->p_this->p_structure->p_hlsRate->addBytes(v5->p_data->i_buffer_index, v5->t_download);
                freeSegment(v5);
                this->p_this->p_structure->p_hlsManager->cleanSegment(v2, v3);
                return;
            }
            freeSegment(v5);
        }
    }
}

int btContent::WriteSegment(size_t i_quality, size_t i_segment_id, block_t *p_segment_data) {
    bool v4; // al@1
    unsigned int v5; // edx@1
    //int v7; // eax@2
    //goalbit_structure_t *v9; // eax@5
    //goalbit_structure_t *v10; // eax@6
    //goalbit_t_0 *v12; // eax@6
    signed int v13; // ebx@8
    long double v14; // fst7@8
    goalbit_event_t_0 *v15; // eax@10
    //goalbit_structure_t *v16; // ebx@10
    //mtime_t v17; // rax@10
    //goalbit_structure_t *v18; // edx@11
    //bool v19; // zf@12
    //goalbit_structure_t *v20; // eax@15
    unsigned int result; // eax@16
    //int v22; // edx@16
    int v24; // eax@19
    int v25; // ebp@21
    metadata_piece_t *v27; // edi@23
    int v28; // ebp@29
    int v29; // ST10_4@30
    char *v30; // eax@30
    char* p_pieces; // [sp+2Ch] [bp-C0h]@18
    array_t<metadata_piece_t> *p_piecesa; // [sp+2Ch] [bp-C0h]@21
    md5_state_t state; // [sp+40h] [bp-ACh]@29
    char psz_downloaded_md5[33]; // [sp+9Bh] [bp-51h]@30
    unsigned __int8 digest[16]; // [sp+BCh] [bp-30h]@29
    //int v36; // [sp+CCh] [bp-20h]@1

    if (this->p_hls_bitfield->IsSet(i_quality, i_segment_id)) {
        return -2;
    }
    this->i_last_recv = mdate();
    if ( p_this->p_structure->b_p2p_enabled && p_this->p_structure->p_metadataManager->isReady() )
    {
        print_dbg(__func__, "[btContent] WriteSegment - check MD5");
        p_pieces = p_this->p_structure->p_metadataManager->getSegmentMD5(i_quality, i_segment_id);
        if ( !p_pieces )
        {
            print_dbg(__func__, "[btContent] WriteSegment - psz_original_md5 = %s (%d)", "NULL", 0);
            goto LABEL_20;
        }
        v24 = strlen(p_pieces);
        print_dbg(__func__, "[btContent] WriteSegment - psz_original_md5 = %s (%d)", p_pieces, v24);
        if ( strlen(p_pieces) != 0x20 )
        {
            LABEL_20:
            print_dbg(__func__, "[btContent] WriteSegment - No MD5 info available");
            goto LABEL_3;
        }
        md5_init(&state);
        md5_append(&state, p_segment_data->p_buffer, p_segment_data->i_buffer_index);
        v28 = 0;
        md5_finish(&state, digest);
        do
        {
            v29 = digest[v28];
            v30 = &psz_downloaded_md5[2 * v28++];
            sprintf(v30, "%02x", v29);
        } while ( v28 != 0x10 );
        psz_downloaded_md5[0x20] = 0;
        if ( !strcmp(psz_downloaded_md5, (const char *)p_pieces) )
        {
            print_msg(__func__, "Same MD5 for segment (%d,%d): %s", i_quality, i_segment_id, psz_downloaded_md5);
        }
        else
        {
            print_err(__func__, "MD5 error for segment %d (quality %d).", i_segment_id, i_quality);
            print_err(__func__, "    Original MD5 = %s", p_pieces);
            print_err(__func__, "    Download MD5 = %s", psz_downloaded_md5);
        }
        free(p_pieces);
    }
    LABEL_3:
    if ( !this->p_cache )
    {
        return -1;
    }
    if ( this->p_cache->WriteSegment(i_quality, i_segment_id, p_segment_data) < 0 )
    {
        print_err(__func__,
                  "[BtContent] WriteSegment - Error writing segment %d for quality %d",
                  i_segment_id,
                  i_quality);
        return -1;
    } else {
        print_dbg(__func__,
                  "[BtContent] WriteSegment - segment %d for quality %d",
                  i_segment_id,
                  i_quality);
    }
    this->p_hls_bitfield->Set(i_quality, i_segment_id);
    //v9 = this->p_this->p_structure;
    if ( p_this->p_structure->b_p2p_enabled )
    {
        v25 = 0;
        p_piecesa = p_this->p_structure->p_metadataManager->getSegmentPieces(i_quality, i_segment_id);
        if ( p_piecesa )
        {
            while ( v25 < array_count(p_piecesa) )
            {
                v27 = array_item_at_index(p_piecesa, v25);
                if ( v27 )
                {
                    print_dbg(goalbitp2p, "[BtContent] WriteSegment - Set piece (#%u,%u) in P2P bitfield ( HLS segment: %u)",
                              v27->i_piece_id,
                              i_quality,
                              i_segment_id);
                    this->p_p2p_bitfield->Set(i_quality, v27->i_piece_id);
                    this->p_this->p_structure->WORLD->Tell_World_I_Have(i_quality, v27->i_piece_id);
                    this->p_this->p_structure->WORLD->CancelPiece(v27->i_piece_id);
                    this->p_this->p_structure->PENDINGQUEUE->DeletePieceByID(v27->i_piece_id);
                    freePiece(v27);
                }
                ++v25;
            }
            array_destroy(p_piecesa);
        }
    }
    UpdateHLSStats(i_quality, i_segment_id);
    update_ABD(true);
    print_dbg(__FUNCTION__, "");
    Mypthread_mutex_lock(29, &this->p_this->p_structure->exec_lock);
    //v10 = this->p_this->p_structure;
    bool b_buffering = p_this->p_structure->b_buffering;
    Mypthread_mutex_unlock(29, &p_this->p_structure->exec_lock);
    //v12 = this->p_this;
    if ( b_buffering )
    {
        v13 = 100;
        v14 = 100.0 * p_this->p_structure->exec_info.f_abd / p_this->p_structure->f_hls_buffer_size;
        if ( v14 <= 100 )
            v13 = v14;
        print_dbg(__func__, "[BTContent] WriteSegment - Buffering: %d %c", v13, '%');
        //            v15 = new goalbit_event_t_0();
        //            v15->g_value.i_int = v13;
        //            v15->i_event = 2;
        //            ReportEvent(this->p_this, v15);//goalbit_event_callback
        p_this->p_param->p_external_ref->SetBufferingState(true);
        p_this->p_param->p_external_ref->SetBufferingPercentage(v13);
        p_this->p_structure->i_last_exec_report = mdate();
        if ( p_this->p_structure->exec_info.f_abd >= p_this->p_structure->f_hls_buffer_size ) {
            Mypthread_mutex_lock(29, &this->p_this->p_structure->exec_lock);
            p_this->p_structure->b_buffering = false;
            bool b_p2p_syncing = p_this->p_structure->b_p2p_syncing;
            Mypthread_mutex_unlock(29, &this->p_this->p_structure->exec_lock);
            if (b_p2p_syncing == false) {
                StartExecution(p_this);
            }
        }
    }
    Mypthread_mutex_lock(30, &p_this->p_structure->exec_lock);
    p_this->p_structure->b_check_win = true;
    Mypthread_mutex_unlock(30, &p_this->p_structure->exec_lock);
    return 0;
}

void btContent::ProcessDownloadedHLSPieces() {

    while ( this->p_this->p_structure->p_hlsManager->hasPiecesReady() ) {
        piece_ready_t *v1 = this->p_this->p_structure->p_hlsManager->getNextPieceReady();
        if ( !v1 || !v1->p_data )
            break;
        print_dbg(__func__,
                  "[btContent] ProcessDownloadedHLSPieces - piece %d ready for quality %d",
                  v1->i_piece,
                  v1->i_stream);
        if ( !WritePiece(v1->i_stream, v1->i_piece, v1->p_data, 1) ) {
            this->p_this->p_structure->WORLD->Tell_World_I_Have(v1->i_stream, v1->i_piece);
            this->p_this->p_structure->WORLD->CancelPiece(v1->i_piece);
            this->p_this->p_structure->PENDINGQUEUE->DeletePieceByID(v1->i_piece);
            this->p_this->p_structure->p_hlsRate->addBytes(v1->p_data->i_buffer_index, v1->t_downloaded);
        }
        block_Release(v1->p_data);
        free(v1);
    }

}

unsigned int btContent::WritePiece(size_t i_piece_quality, size_t i_piece_id, block_t *p_piece_data,
                                   bool b_safe_content) {
    int v6; // eax@2
    char *v7; // eax@3
    int v9; // ebx@5
    int b_should_write; // ST10_4@6
    char *v11; // eax@6
    btCache *v12; // eax@9
    //goalbit_structure_t *v13; // eax@11
    //int v14; // edx@12
    char *psz_original_md5; // [sp+20h] [bp-BCh]@3
    md5_state_t state; // [sp+30h] [bp-ACh]@5
    char psz_downloaded_md5[33]; // [sp+8Bh] [bp-51h]@6
    unsigned __int8 digest[16]; // [sp+ACh] [bp-30h]@5
    //int v21; // [sp+BCh] [bp-20h]@1

    if ( this->p_p2p_bitfield->IsSet(i_piece_quality, i_piece_id) ) {
        return -2;
    }
    v6 = this->p_this->p_structure->p_metadataManager->getPieceSegment(i_piece_id);
    if ( v6 == 0x80000001 ) {
        return -1;
    }
    v7 = this->p_this->p_structure->p_metadataManager->getPieceMD5(i_piece_quality, v6, i_piece_id);
    psz_original_md5 = v7;
    if ( !v7 )
    {
        print_dbg(__func__, "[btContent] WritePiece - psz_original_md5 = %s (%d)", "NULL", 0);
        goto LABEL_16;
    }
    print_dbg(__func__, "[btContent] WritePiece - psz_original_md5 = %s (%d)", psz_original_md5, strlen(v7));
    if ( strlen(psz_original_md5) != 0x20 )
    {
        LABEL_16:
        print_err(__func__,
                  "[BTCONTENT] Wrong MD5 info! can't check MD5 piece (%d,%d,%d)...",
                  i_piece_quality,
                  v6,
                  i_piece_id);
        return -1;
    }
    md5_init(&state);
    md5_append(&state, p_piece_data->p_buffer, p_piece_data->i_buffer_index);
    v9 = 0;
    md5_finish(&state, digest);
    do
    {
        b_should_write = digest[v9];
        v11 = &psz_downloaded_md5[2 * v9++];
        sprintf(v11, "%02x", b_should_write);
    } while ( v9 != 0x10 );
    psz_downloaded_md5[0x20] = 0;
    if ( !strcmp(psz_downloaded_md5, psz_original_md5) ) {
        print_msg(__func__,
                  "[BTCONTENT] Same MD5 for piece (%d,%d): %s",
                  i_piece_quality,
                  i_piece_id,
                  psz_downloaded_md5);
    }
    else
    {
        print_err(__func__, "[BTCONTENT] MD5 error for piece %d (quality %d).", i_piece_id, i_piece_quality);
        print_err(__func__, "[BTCONTENT]     Original MD5 = %s", psz_original_md5);
        print_err(__func__, "[BTCONTENT]     Download MD5 = %s", psz_downloaded_md5);
        if ( !b_safe_content ) {
            return -3;
        }
    }
    free(psz_original_md5);
    v12 = this->p_cache;
    if ( v12 && v12->AddPiece(p_piece_data, i_piece_quality, i_piece_id, 1) >= 0 ) {
        this->p_p2p_bitfield->Set(i_piece_quality, i_piece_id);
        checkSegmentCompletion(i_piece_quality, v6, i_piece_id);
        update_ABD(true);
        print_dbg(__FUNCTION__, "");
        UpdateP2PStats(i_piece_quality, v6, i_piece_id);
        Mypthread_mutex_lock(29, &this->p_this->p_structure->exec_lock);
        //v10 = this->p_this->p_structure;
        bool b_buffering = p_this->p_structure->b_buffering;
        Mypthread_mutex_unlock(29, &p_this->p_structure->exec_lock);
        //v12 = this->p_this;
        if ( b_buffering )
        {
            int v13 = 100;
            int v14 = 100.0 * p_this->p_structure->exec_info.f_abd / p_this->p_structure->f_hls_buffer_size;
            if ( v14 <= 100 )
                v13 = v14;
            print_dbg(__func__, "[BTContent] WriteSegment - Buffering: %d %c", v13, '%');
            //            v15 = new goalbit_event_t_0();
            //            v15->g_value.i_int = v13;
            //            v15->i_event = 2;
            //            ReportEvent(this->p_this, v15);//goalbit_event_callback
            p_this->p_param->p_external_ref->SetBufferingState(true);
            p_this->p_param->p_external_ref->SetBufferingPercentage(v13);
            p_this->p_structure->i_last_exec_report = mdate();
            if ( p_this->p_structure->exec_info.f_abd >= p_this->p_structure->f_hls_buffer_size ) {
                Mypthread_mutex_lock(29, &this->p_this->p_structure->exec_lock);
                p_this->p_structure->b_buffering = false;
                bool b_p2p_syncing = p_this->p_structure->b_p2p_syncing;
                Mypthread_mutex_unlock(29, &this->p_this->p_structure->exec_lock);
                if (b_p2p_syncing == false) {
                    StartExecution(p_this);
                }
            }
        }
        Mypthread_mutex_lock(31, &this->p_this->p_structure->exec_lock);
        p_this->p_structure->b_check_win = true;
        Mypthread_mutex_unlock(31, &p_this->p_structure->exec_lock);
        return 0;
    }
    return -1;
}

void
btContent::checkSegmentCompletion(size_t i_piece_quality, size_t i_segment_id, size_t i_piece_id) {
    int v4; // ebp@2
    bool v5; // di@2
    metadata_piece_t *v6; // eax@4
    metadata_piece_t *v7; // edi@4
    bool v8; // zf@5
    int v9; // eax@5
    int v10; // edx@5
    array_t<metadata_piece_t> *p_pieces; // [sp+28h] [bp-24h]@1

    p_pieces = this->p_this->p_structure->p_metadataManager->getSegmentPieces(
            i_piece_quality,
            i_segment_id);
    if ( p_pieces )
    {
        v4 = 0;
        v5 = 1;
        print_dbg(__func__,
                  "[btContent] checkSegmentCompletion - piece received: %d (%d,%d)",
                  i_piece_id,
                  i_segment_id,
                  i_piece_quality);
        while ( 1 )
        {
            if ( v4 >= array_count(p_pieces) )
            {
                freePiecesArray(p_pieces);
                if ( !v5 )
                    return;
                goto LABEL_10;
            }
            if ( !v5 )
                break;
            v6 = array_item_at_index(p_pieces, v4);
            v7 = v6;
            if ( !v6 )
            {
                freePiecesArray(p_pieces);
                LABEL_10:
                print_dbg(__func__,
                          "[btContent] checkSegmentCompletion - segment completed! %d (%d)",
                          i_segment_id,
                          i_piece_quality);
                this->i_last_recv = mdate();
                this->p_hls_bitfield->Set(i_piece_quality, i_segment_id);
                return;
            }
            v8 = this->p_p2p_bitfield->IsSet(i_piece_quality, v6->i_piece_id) == 0;
            v9 = v7->i_piece_id;
            v10 = v7->i_piece_id;
            if ( v8 )
                print_dbg(__func__, "[btContent] checkSegmentCompletion - piece %d (%d) set?: NO", v9, v10);
            else
                print_dbg(__func__, "[btContent] checkSegmentCompletion - piece %d (%d) set?: YES", v9, v10);
            ++v4;
            v5 = this->p_p2p_bitfield->IsSet(i_piece_quality, v7->i_piece_id);
        }
        freePiecesArray(p_pieces);
    }
}

void btContent::update_ABD(bool b_new_content)
{
    //goalbit_structure_t *v2; // eax@1
    bool b_buffering; // bp@1
    int i_target_quality; // edi@1
    size_t i_new_abi; // esi@1
    float_t f_segment_played; // fst7@1
    float_t f_new_abd; // fst7@4
    //float v8; // ST20_4@5
    //goalbit_structure_t *v9; // eax@5
    unsigned int i_max_segment_id; // eax@6
    unsigned int v12; // esi@7
    float_t v18; // fst7@18
    float_t v19; // fst6@18
    //float v20; // [sp+20h] [bp-5Ch]@1
    mtime_t i_played_time; // [sp+38h] [bp-44h]@1
    float i_played_timea; // [sp+38h] [bp-44h]@6
    float f_new_segment_played; // [sp+48h] [bp-34h]@1
    size_t i_segment_id; // [sp+4Ch] [bp-30h]@1
    float f_abd; // [sp+58h] [bp-24h]@1

    Mypthread_mutex_lock(32, &this->p_this->p_structure->exec_lock);
    i_segment_id = p_this->p_structure->exec_info.i_segment_id;
    f_abd = p_this->p_structure->exec_info.f_abd;
    b_buffering = p_this->p_structure->b_buffering;
    i_target_quality = p_this->p_structure->i_target_quality;
    i_new_abi = p_this->p_structure->exec_info.i_abi;
    f_segment_played = p_this->p_structure->exec_info.f_segment_played;
    i_played_time = p_this->p_structure->exec_info.i_played_time;
    Mypthread_mutex_unlock(32, &p_this->p_structure->exec_lock);

    f_new_segment_played = f_segment_played;
    if ( !b_buffering && i_played_time ) {
        float v15 = (mdate() - i_played_time) / 1000000.0;
        float v16 = p_this->p_structure->p_hlsManager->getSegmentDuration(i_target_quality, i_segment_id);
        f_new_segment_played = v15;
        if ( v15 > v16 )
            f_new_segment_played = v16;
        //v6 = v20;
    }
    if ( b_new_content ) {
        i_played_timea = 0;
        i_max_segment_id = p_this->p_structure->p_hlsManager->getMaxSegmentID(i_target_quality);
        i_new_abi = i_segment_id;
        while (i_new_abi <= i_max_segment_id) {
            if ( !p_hls_bitfield->IsSet(0x80000001, i_new_abi) ) {
                i_played_timea += getPlayableDurationFromSegment(i_new_abi);
                break;
            }
            i_played_timea += this->p_this->p_structure->p_hlsManager->getSegmentDuration(i_target_quality, i_new_abi);
            i_new_abi++;
        }
        f_new_abd = i_played_timea - f_new_segment_played;
        if ( f_new_abd < 0.0 )
            f_new_abd = 0;
    }
    else
    {
        f_new_abd = f_abd - (f_new_segment_played - f_segment_played);
    }
    //v8 = f_new_abd;
    Mypthread_mutex_lock(33, &this->p_this->p_structure->exec_lock);
    //v9 = this->p_this->p_structure;
    p_this->p_structure->exec_info.f_abd = f_new_abd;
    p_this->p_structure->exec_info.f_segment_played = f_new_segment_played;
    p_this->p_structure->exec_info.i_abi = i_new_abi;
    Mypthread_mutex_unlock(33, &p_this->p_structure->exec_lock);
    print_dbg(__func__, "[BTContent] update_ABD - abd: %0.2f, exec: %u, abi: %u", f_new_abd, i_segment_id, i_new_abi);
    return;
}

long double btContent::getPlayableDurationFromSegment(size_t i_segment_id) {
    //goalbit_structure_t *v2; // edx@1
    //MetadataManager *v3; // eax@1
    size_t v4; // ebp@4
    array_t<metadata_keyframe_t> *v5; // eax@5
    int v6; // ebx@8
    metadata_piece_t *v7; // esi@10
    int v8; // esi@14
    metadata_keyframe_t * v9; // eax@16
    //unsigned int v10; // ebx@16
    //goalbit_structure_t *v12; // eax@20
    uint64_t i_downloaded_offset; // [sp+18h] [bp-34h]@8
    float f_playable_dur; // [sp+24h] [bp-28h]@1
    array_t<metadata_keyframe_t> *p_keyframes; // [sp+28h] [bp-24h]@5
    array_t<metadata_piece_t> *p_pieces; // [sp+2Ch] [bp-20h]@7

    f_playable_dur = 0.0;
    //v2 = this->p_this->p_structure;
    //v3 = p_this->p_structure->p_metadataManager;
    if ( p_this->p_structure->p_metadataManager && p_this->p_structure->b_p2p_enabled && p_this->p_structure->i_hls_qualities )
    {
        v4 = 0;
        while ( 1 )
        {
            v5 = p_this->p_structure->p_metadataManager->getSegmentKeyframes( v4, i_segment_id);
            p_keyframes = v5;
            if ( !v5 )
                goto LABEL_20;
            if ( array_count(v5) )
            {
                p_pieces = this->p_this->p_structure->p_metadataManager->getSegmentPieces(v4, i_segment_id);
                if ( p_pieces )
                    break;
            }
            ++v4;
            freeKeyframesArray(p_keyframes);
            //v12 = this->p_this->p_structure;
            if ( p_this->p_structure->i_hls_qualities <= v4 )
                return (float)0.0;
            LABEL_21:;
            //v3 = p_this->p_structure->p_metadataManager;
        }
        v6 = 0;
        i_downloaded_offset = 0LL;
        if ( array_count(p_pieces) )
        {
            while ( v6 < array_count(p_pieces) )
            {
                v7 = array_item_at_index(p_pieces, v6);
                if ( !this->p_p2p_bitfield->IsSet(v4, v7->i_piece_id) )
                    break;
                i_downloaded_offset += v7->i_duration;
                ++v6;
            }
            if ( i_downloaded_offset )
            {
                v8 = 0;
                for ( f_playable_dur = 0.0; v8 < array_count(p_keyframes); f_playable_dur = v9->i_duration )
                {
                    v9 = array_item_at_index(p_keyframes, v8);
                    if ( v9->i_offset > i_downloaded_offset )
                    {
                        break;
                    }
                    ++v8;
                }
                freeKeyframesArray(p_keyframes);
                freePiecesArray(p_pieces);
                return f_playable_dur;
            }
        }
        freeKeyframesArray(p_keyframes);
        freePiecesArray(p_pieces);
        LABEL_20:
        ++v4;
        //v12 = this->p_this->p_structure;
        if ( p_this->p_structure->i_hls_qualities <= v4 )
            return (float)0.0;
        goto LABEL_21;
    }
    return f_playable_dur;
}

void btContent::UpdateP2PStats(size_t i_quality, size_t i_segment_id, size_t i_piece_id) {
    //goalbit_structure_t *v4; // eax@1
    metadata_piece_t *v5; // eax@3
    int v6; // edx@4
    int v7; // edi@5
    //__int64 v8; // rcx@6
    long double v9; // fst7@6
    unsigned int v10; // ST20_4@6
    float v11; // ST3C_4@6
    //goalbit_structure_t *v12; // ecx@7
    unsigned int v13; // eax@7
    unsigned int v14; // edi@7
    unsigned int v15; // ebp@7
    int v16; // ebx@7
    long double v17; // fst7@9
    long double v18; // fst6@9
    long double v19; // fst5@9
    long double v20; // fst4@10
    long double v21; // t0@10
    long double v22; // fst4@10
    //goalbit_structure_t *v23; // ecx@12
    int v24; // ebx@12
    //goalbit_structure_t *v25; // [sp+18h] [bp-44h]@4
    //goalbit_t_0 *p_goalbit; // [sp+28h] [bp-34h]@4

    //v4 = this->p_this->p_structure;
    if ( p_this->p_structure->b_p2p_enabled )
    {
        v5 = p_this->p_structure->p_metadataManager->getPiece(i_quality, i_segment_id, i_piece_id);
        if ( v5 )
        {
            //p_goalbit = this->p_this;
            v6 = this->p_this->p_structure->i_history_count;
            //v25 = this->p_this->p_structure;
            if ( v6 > 9 )
            {
                //v23 = this->p_this->p_structure;
                v24 = 0;
                v7 = v6 - 1;
                do
                {
                    ++v24;
                    p_this->p_structure->f_p2p_bitrates[v24-1] = p_this->p_structure->f_p2p_bitrates[v24];
                    p_this->p_structure->f_p2p_lengths[v24-1] = p_this->p_structure->f_p2p_lengths[v24];
                    p_this->p_structure->f_p2p_durations[v24-1] = p_this->p_structure->f_p2p_durations[v24];
                }
                while ( v24 != v7 );
            }
            else
            {
                v7 = this->p_this->p_structure->i_history_count;
                p_this->p_structure->i_history_count = ++v6;
            }
            //HIDWORD(v8) = HIDWORD(v5->i_size);
            v9 = v5->i_duration;
            v10 = v5->i_size;
            //LODWORD(v8) = v5->i_size;
            v11 = 8 * v5->i_size;
            p_this->p_structure->f_p2p_bitrates[v7] = (v11 / v9);
            p_this->p_structure->f_p2p_lengths[v7] = v10;
            p_this->p_structure->f_p2p_durations[v7] = (signed int)(v9 * 1000000.0);
            if ( v6 <= 0 )
            {
                v17 = 0.0;
                v18 = 0.0;
                v19 = 0.0;
            }
            else
            {
                //v12 = v25;
                v13 = 0;
                v14 = 0;
                v15 = 0;
                v16 = 0;
                do
                {
                    v15 += p_this->p_structure->f_p2p_bitrates[v16];
                    v14 += p_this->p_structure->f_p2p_lengths[v16];
                    v13 += p_this->p_structure->f_p2p_durations[v16];
                    ++v16;
                } while ( v16 != v6 );
                v17 = (long double)v15;
                v18 = (long double)v14;
                v19 = (long double)v13;
            }
            v20 = (long double)v6;
            v21 = v20;
            v22 = v17 / v20;
            p_this->p_structure->f_avg_p2p_bitrate = v22;
            p_this->p_structure->f_avg_p2p_length = v18 / v21;
            p_this->p_structure->f_avg_usec_x_piece = v19 / v21;
            p_this->p_structure->f_avg_piece_x_sec = v22 / (v18 / v21);
            if ( i_piece_id == 0xA * (i_piece_id / 0xA) )
            {
                print_msg(__func__, "**********************************************************");
                print_msg(__func__, "Average values: ");
                print_msg(__func__,
                          " - f_avg_p2p_bitrate   = %f (bits per second)",
                          this->p_this->p_structure->f_avg_p2p_bitrate);
                print_msg(__func__,
                          " - f_avg_p2p_length    = %f (bytes)",
                          this->p_this->p_structure->f_avg_p2p_length);
                print_msg(__func__,
                          " - f_avg_usec_x_piece  = %f (microseconds)",
                          this->p_this->p_structure->f_avg_usec_x_piece);
                print_msg(__func__,
                          " - f_avg_piece_x_sec   = %f (segments per seconds)",
                          this->p_this->p_structure->f_avg_piece_x_sec);
                print_msg(__func__, "**********************************************************");
            }
        }
    }

}

void btContent::ExecutionMonitor() {
    //btContent *v1; // ebx@1
    //goalbit_structure_t *v2; // eax@2
    bool b_buffering; // si@2
    unsigned int i_segment_id; // ebp@2
    //goalbit_t_0 *v5; // eax@2
    //goalbit_structure_t *v6; // edx@2
    //float_t v7; // esi@3
    //long double v8; // fst7@5
    //long double v11; // fst7@12
    //btSelfBitField *v12; // eax@12
    //goalbit_structure_t *v13; // eax@14
    size_t idx_4; // ST0C_4@16
    //unsigned int v17; // edi@21
    int64_t v20; // esi@21
    //goalbit_structure_t *v21; // eax@21
    float_t v22; // esi@25
    //mtime_t v24; // rax@33
    //goalbit_t_0 *v25; // eax@35
    //goalbit_structure_t *v26; // edx@35
    //bool v27; // zf@35
    int v28; // edi@39
    //goalbit_t_0 **v29; // esi@39
    metadata_piece_t *v30; // ebx@42
    int v31; // esi@49
    size_t v32; // edi@52
    metadata_piece_t *v34; // eax@54
    int v38; // edi@64
    array_t<metadata_piece_t> *v39; // ebp@64
    //btContent *v40; // esi@65
    metadata_piece_t *v41; // ebx@68
    float_t f_abd; // [sp+2Ch] [bp-50h]@2
    float_t f_cotent_to_request2; // [sp+2Ch] [bp-50h]@3
    int i_cotent_to_requestb; // [sp+2Ch] [bp-50h]@49
    float f_segment_played; // [sp+30h] [bp-4Ch]@2
    //float f_reques_pieces2; // [sp+30h] [bp-4Ch]@5
    bool b_reques_piecesb; // [sp+30h] [bp-4Ch]@21
    size_t target_quality; // [sp+40h] [bp-3Ch]@2
    float f_buffer_durationa; // [sp+40h] [bp-3Ch]@12
    unsigned int i_hlsMaxSegment_id; // [sp+44h] [bp-38h]@2
    int i_segment_ida; // [sp+44h] [bp-38h]@53
    array_t<metadata_piece_t> *p_missing_piecesa; // [sp+48h] [bp-34h]@37
    bool b_download_piece; // [sp+53h] [bp-29h]@38

    //v1 = this;
    if ( this->p_this->p_structure->b_p2p_syncing ) {
//#ifndef bDisabletimecheckforDebug
        if (mdate() - this->p_this->p_structure->i_p2p_sync_start > 6000000) {
            print_dbg(__func__, "[BTContent] Canceling the P2P manifest syncing");
            p_this->p_structure->b_p2p_syncing = false;
            if (p_this->p_structure->b_buffering == false) {
                StartExecution(p_this);
            }
        }
//#endif
    }

    update_ABD(false);
    print_dbg(__FUNCTION__, "");

    Mypthread_mutex_lock(34, &this->p_this->p_structure->exec_lock);
    f_segment_played = p_this->p_structure->exec_info.f_segment_played;
    b_buffering = p_this->p_structure->b_buffering;
    i_segment_id = p_this->p_structure->exec_info.i_segment_id;
    target_quality = p_this->p_structure->i_target_quality;
    f_abd = p_this->p_structure->exec_info.f_abd;
    float_t f_hls_buffer_size = p_this->p_structure->f_hls_urgent_size;
    Mypthread_mutex_unlock(34, &p_this->p_structure->exec_lock);

    i_hlsMaxSegment_id = this->p_this->p_structure->p_hlsManager->getMaxSegmentID(target_quality);
    //v5 = this->p_this;
    //v6 = this->p_this->p_structure;
    //v7 = this->p_this->p_structure->f_hls_buffer_size;
    if ( b_buffering || f_hls_buffer_size > f_abd ) {
        if (b_buffering) {
            f_cotent_to_request2 = 0.0;
        } else {
            f_cotent_to_request2 = 0.0 - f_segment_played;
        }
        while (i_hlsMaxSegment_id >= i_segment_id && f_cotent_to_request2 <= p_this->p_structure->f_hls_buffer_size) {
            f_buffer_durationa = p_this->p_structure->p_hlsManager->getSegmentDuration(target_quality, i_segment_id);
            if (p_hls_bitfield && p_hls_bitfield->IsSet(0x80000001, i_segment_id) || p_this->p_structure->p_hlsManager->isSegmentRequestedOrReady(i_segment_id)) {
                f_cotent_to_request2 += f_buffer_durationa;
                i_segment_id++;
                continue;
            }
            if (p_this->p_structure->b_p2p_enabled && p_this->p_structure->b_hls_can_down_pieces) {
                p_missing_piecesa = this->GetMissingPiecesInSegment(target_quality, i_segment_id);
                if (p_missing_piecesa) {
                    b_download_piece = 0;
                    if (array_count(p_missing_piecesa) <= 2) {
                        v28 = 0;
                        while (v28 < array_count(p_missing_piecesa)) {
                            v30 = array_item_at_index(p_missing_piecesa, v28);
                            if (!this->p_this->p_structure->p_hlsManager->isPieceRequestedOrReady(v30->i_piece_id)) {
                                this->p_this->p_structure->p_hlsManager->downloadPiece(target_quality, v30->i_piece_id, i_segment_id, v30->i_offset, v30->i_size);
                                print_dbg(__func__, "[BTCONTENT] ExecutionMonitor - Requesting piece (#%u,%u)", target_quality, v30->i_piece_id);
                                b_download_piece = 1;
                            }
                            ++v28;
                        }
                        f_cotent_to_request2 = f_cotent_to_request2 + f_buffer_durationa;
                        if (b_download_piece) {
                            print_dbg(__func__, "[BTCONTENT] ExecutionMonitor - Requested buffer duration: %f", f_cotent_to_request2);
                        }
                        freePiecesArray(p_missing_piecesa);
                        i_segment_id++;
                        continue;
                    }
                    freePiecesArray(p_missing_piecesa);
                }
            }
            p_this->p_structure->p_hlsManager->downloadSegment(target_quality, i_segment_id);
            i_segment_id++;
            f_cotent_to_request2 += f_buffer_durationa;
            print_dbg(__func__, "[BTCONTENT] ExecutionMonitor - Requesting segment (#%u,%u)", target_quality, i_segment_id);
            print_dbg(__func__, "[BTCONTENT] ExecutionMonitor - Requested buffer duration: %f", f_cotent_to_request2);
        }
        return;
    }
    if ( (mdate() - this->i_last_opt_download) <= 1000000 * GetOptimisticDownInterval()) {
        return;
    }
    v20 = 1;
    this->i_last_opt_download = this->i_last_opt_download + 1000000 * (random()%2);
    print_dbg(__func__, "[btContent] ExecutionMonitor - It's time for an optimistic download");
    b_reques_piecesb = false;
    //v21 = this->p_this->p_structure;
    if ( p_this->p_structure->b_p2p_enabled && p_this->p_structure->b_hls_can_down_pieces )
    {
        v20 = p_this->p_structure->WORLD->m_peers_count < 1 ? 1 : 0;
        b_reques_piecesb = p_this->p_structure->WORLD->m_peers_count >= 1;
    }
    //p_missing_pieces = (array_t *)malloc(0x24u);
    btBitField *p_missing_pieces = new btBitField(this->p_this, 1u, v20);
    if ( i_segment_id <= i_hlsMaxSegment_id )
    {
        v22 = i_segment_id;
        do
        {
            if ( (!this->p_hls_bitfield || !this->p_hls_bitfield->IsSet(0x80000001, v22))
                 && !this->p_this->p_structure->p_hlsManager->isSegmentRequestedOrReady(v22) )
            {
                if ( b_reques_piecesb )
                {
                    v38 = 0;
                    v39 = this->GetMissingPiecesInSegment(target_quality, v22);
                    if ( v39 )
                    {
                        while ( v38 < array_count(v39) )
                        {
                            v41 = array_item_at_index(v39, v38);
                            if ( !this->p_this->p_structure->p_hlsManager->isPieceRequestedOrReady(v41->i_piece_id) && !this->p_this->p_structure->WORLD->AlreadyRequested(0x80000001, v41->i_piece_id) )
                            {
                                p_missing_pieces->Set(0, v41->i_piece_id);
                            }
                            ++v38;
                        }
                        freePiecesArray(v39);
                    }
                }
                else if ( !this->p_this->p_structure->b_p2p_enabled )
                {
                    p_missing_pieces->Set(0, v22);
                }
            }
        } while ( ++v22 <= i_hlsMaxSegment_id );
    }
    v31 = 0;

    i_cotent_to_requestb = 2 - (b_reques_piecesb < 1u);
    do
    {
        if ( !p_missing_pieces->IsEmpty(0) )
        {
            v32 = p_missing_pieces->SelectRandomUniformID(0);
            p_missing_pieces->UnSet(0, v32);
            this->i_last_opt_download = mdate();
            if ( b_reques_piecesb )
            {
                i_segment_ida = this->p_this->p_structure->p_metadataManager->getPieceSegment(v32);
                if ( i_segment_ida != 0x80000001 )
                {
                    v34 = this->p_this->p_structure->p_metadataManager->getPiece(target_quality, i_segment_ida, v32);
                    if ( v34 )
                    {
                        print_dbg(__func__, "[btContent] ExecutionMonitor - Optimistic download for piece (#%u,%u) from segment #%u", target_quality, v32, i_segment_ida);
                        this->p_this->p_structure->p_hlsManager->downloadPiece(target_quality, v34->i_piece_id, i_segment_ida, v34->i_offset, v34->i_size);
                    }
                }
            } else {
                print_dbg(__func__,
                          "[btContent] ExecutionMonitor - Optimistic download for segment (#%u,%u)",
                          target_quality,
                          v32);
                this->p_this->p_structure->p_hlsManager->downloadSegment(target_quality, v32);
            }
        }
        ++v31;
    } while ( i_cotent_to_requestb > v31 );
    if ( p_missing_pieces )
    {
        delete(p_missing_pieces);
    }
}

metadata_piece_t* copyPiece(const metadata_piece_t *p_piece)
{
    metadata_piece_t *v1; // esi@1
    metadata_piece_t *v2; // eax@2
    //int v3; // edx@3
    int v4; // eax@3
    //int v5; // edx@3
    //int v6; // eax@3

    v1 = 0;
    if ( p_piece )
    {
        v2 = new metadata_piece_t();
        v1 = v2;
        if ( v2 )
        {
            //v3 = HIDWORD(p_piece->i_offset);
            v2->i_piece_id = p_piece->i_piece_id;
            v2->i_duration = p_piece->i_duration;
            v4 = p_piece->i_offset;
            //HIDWORD(v1->i_offset) = v3;
            v1->i_offset = p_piece->i_offset;
            //v5 = HIDWORD(p_piece->i_size);
            //LODWORD(v1->i_offset) = v4;
            //v6 = p_piece->i_size;
            //HIDWORD(v1->i_size) = v5;
            //LODWORD(v1->i_size) = v6;
            v1->i_size = p_piece->i_size;
            v1->psz_MD5 = (char *)strdup(p_piece->psz_MD5);
        }
    }
    return v1;
}

template <typename T>
void array_insert(array_t<T> *p_array, T *p_elem, int i_index)
{
    int v3; // esi@1
    T** v4; // eax@2
    int v5; // ebp@3

    v3 = p_array->i_count;
    if ( p_array->i_count <= 0 )
    {
        v4 = (T**)malloc(sizeof(T*));
        p_array->pp_elems = v4;
    }
    else
    {
        v4 = (T**)realloc(p_array->pp_elems, sizeof(T*) * (v3 + 1));
        v3 = p_array->i_count;
        p_array->pp_elems = v4;
    }
    v5 = i_index;
    if ( v3 - i_index > 0 )
    {
        memmove(&v4[i_index + 1], &v4[v5], 4 * (v3 - i_index));
        v3 = p_array->i_count;
        v4 = p_array->pp_elems;
    }
    v4[v5] = p_elem;
    p_array->i_count = v3 + 1;
}

template <typename T>
void array_append(array_t<T> *p_array, T* p_elem)
{
    array_insert(p_array, p_elem, p_array->i_count);
}

array_t<metadata_piece_t> *btContent::GetMissingPiecesInSegment(size_t i_quality, size_t i_segment_id) {

    array_t<metadata_piece_t> *p_missing_pieces = 0;
    if ( p_this->p_structure->b_p2p_enabled )
    {
        array_t<metadata_piece_t> *v4 = p_this->p_structure->p_metadataManager->getSegmentPieces(i_quality, i_segment_id);
        if ( v4 )
        {
            int v5 = 0;
            p_missing_pieces = array_new<metadata_piece_t>();
            while ( v5 < array_count(v4) )
            {
                const metadata_piece_t *v6 = array_item_at_index(v4, v5);
                if ( !this->p_p2p_bitfield->IsSet(i_quality, v6->i_piece_id) )
                {
                    array_append(p_missing_pieces, copyPiece(v6));
                }
                ++v5;
            }
            freePiecesArray(v4);
        }
    }
    return p_missing_pieces;
}

void btContent::SyncWindows() {
    int v1 = this->p_this->p_structure->p_hlsManager->getInitialSequence();
    if ( v1 >= 0 ) {
        define_HLSWindowOffset(this->p_this, v1);
        UpdateHLSWindow(this->p_this->p_structure->i_hls_win_offset);
        if ( p_this->p_structure->b_p2p_enabled ) {
            if ( p_this->p_structure->p_metadataManager ) {
                if ( p_this->p_structure->p_metadataManager->isReady() ) {
                    int v5 = this->p_this->p_structure->p_metadataManager->segmentID2PieceID(v1, 0x80000001, 2, 1);
                    if ( v5 >= 0 ) {
                        define_P2PWindowOffset(this->p_this, v5);
                        UpdateP2PWindow(this->p_this->p_structure->i_p2p_win_offset);
                        this->p_cache->UpdateWindow();
                    }
                }
            }
        }
    }
}

void btContent::UpdateP2PWindow(size_t i_new_p2p_offset) {
    this->p_p2p_bitfield->UpdateWindow(i_new_p2p_offset);
    this->p_this->p_structure->WORLD->UpdatePeersWindow(i_new_p2p_offset);
    this->p_this->p_structure->WORLD->CheckPeersRequest();
}

void btContent::CheckP2PBitFieldFromHLS(btBitField *bf) {
    quality_segmentid *v2; // eax@3
    int v3; // ebx@3
    size_t v4; // edi@3
    int i_segment_id; // ST08_4@3
    int v6; // ebx@3
    array_t<metadata_piece_t> *v7; // esi@3
    int j; // ST04_4@5
    metadata_piece_t *v9; // ST10_4@5
    //goalbit_structure_t *v10; // eax@10
    int v11; // ebx@12
    array_t<quality_segmentid> *v12; // esi@12
    quality_segmentid *v14; // eax@14
    int i_piece_quality; // ST14_4@14
    int v16; // ST18_4@14
    int i_piece_id; // [sp+18h] [bp-24h]@1
    array_t<quality_segmentid> *p_segments; // [sp+1Ch] [bp-20h]@1

    i_piece_id = 0;
    p_segments = this->p_this->p_structure->p_hlsManager->getSegmentsReadyAndRequested();
    if ( p_segments ) {
        while ( i_piece_id < array_count(p_segments) )
        {
            v2 = array_item_at_index(p_segments, i_piece_id);
            v3 = v2->i_segment_id;
            v4 = v2->i_quality;
            free(v2);
//            print_dbg(__func__,
//                      "[btContent] CheckP2PBitFieldFromHLS - we have requested thru HLS the segment (#%u,%u)",
//                      v3,
//                      v4);
            i_segment_id = v3;
            v6 = 0;
            v7 = this->p_this->p_structure->p_metadataManager->getSegmentPieces(v4, i_segment_id);
            if ( v7 )
            {
                while ( v6 < array_count(v7) )
                {
                    j = v6++;
                    v9 = array_item_at_index(v7, j);
//                    print_dbg(__func__,
//                              "[btContent] CheckP2PBitFieldFromHLS - we have requested thru HLS the piece (#%u,%u)",
//                              v9->i_piece_id,
//                              v4);
                    bf->UnSet(v4, v9->i_piece_id);
                }
                freePiecesArray(v7);
            }
            ++i_piece_id;
        }
        array_destroy(p_segments);
    }
    //v10 = this->p_this->p_structure;
    if ( p_this->p_structure->b_hls_can_down_pieces )
    {
        v11 = 0;
        v12 = p_this->p_structure->p_hlsManager->getPiecesReadyAndRequested();
        if ( v12 )
        {
            while ( v11 < array_count(v12) )
            {
                v14 = array_item_at_index(v12, v11);
                i_piece_quality = v14->i_quality;
                v16 = v14->i_segment_id;
                free(v14);
//                print_dbg(__func__,
//                          "[btContent] CheckP2PBitFieldFromHLS - we have requested thru HLS the piece (#%u,%u)",
//                          v16,
//                          i_piece_quality);
                bf->UnSet(i_piece_quality, v16);
                v11++;
            }
            array_destroy(v12);
        }
    }
}

unsigned int btContent::GetPieceSize(size_t i_piece_quality, size_t i_piece_id) {
    unsigned int v3; // esi@1
    //MetadataManager *v4; // eax@1
    int v5; // eax@2

    v3 = -1;
    //v4 = this->p_this->p_structure->p_metadataManager;
    if ( p_this->p_structure->p_metadataManager )
    {
        v5 = p_this->p_structure->p_metadataManager->getPieceSegment(i_piece_id);
        if ( v5 != 0x80000001 )
            v3 = this->p_this->p_structure->p_metadataManager->getPieceSize(i_piece_quality, v5, i_piece_id);
    }
    return v3;
}

int btContent::GetABIMaxPiece() {
    if ( p_this->p_structure->b_p2p_enabled ) {
        Mypthread_mutex_lock(35, &p_this->p_structure->exec_lock);
        int v4 = p_this->p_structure->exec_info.i_abi;
        Mypthread_mutex_unlock(35, &p_this->p_structure->exec_lock);
        return this->p_this->p_structure->p_metadataManager->segmentID2PieceID(v4, 0x80000001, 3, 2);
    }
    return 0x80000001;
}

void btContent::UpdateHLSStats(size_t i_quality, size_t i_segment_id) {
    hls_segment_t *v3; // eax@3
    //goalbit_structure_t *v4; // edx@4
    signed int v5; // edi@4
    int v6; // edi@6
    unsigned int v7; // esi@6
    //goalbit_structure_t *v9; // eax@6
    //goalbit_structure_t *v10; // edx@6
    signed int v11; // eax@6
    //goalbit_structure_t *v12; // ecx@7
    int v13; // edi@7
    unsigned int v14; // edx@7
    unsigned int v15; // esi@7
    unsigned int v16; // ebp@7
    long double v17; // fst7@9
    long double v18; // fst6@9
    long double v19; // fst5@9
    long double v20; // fst4@10
    long double v21; // t0@10
    long double v22; // fst4@10
    bool v23; // zf@10
    int v24; // edi@13
    int v25; // ecx@13
    int v26; // esi@16
    array_t<metadata_piece_t> *v27; // edi@16
    metadata_piece_t *v28; // eax@18
    //goalbit_structure_t *v30; // [sp+24h] [bp-38h]@6

    if ( this->p_this->p_structure->p_hlsManager->isReady() )
    {
        v3 = this->p_this->p_structure->p_hlsManager->getSegment(i_quality, i_segment_id);
        if ( v3 )
        {
            //v4 = this->p_this->p_structure;
            v5 = p_this->p_structure->i_history_count;
            if ( v5 > 9 )
            {
                v24 = v5 - 1;
                v25 = 0;
                do
                {
                    p_this->p_structure->f_hls_bitrates[v25] = p_this->p_structure->f_hls_bitrates[v25+1];
                    p_this->p_structure->f_hls_lengths[v25] = p_this->p_structure->f_hls_lengths[v25+1];
                    p_this->p_structure->f_hls_durations[v25] = p_this->p_structure->f_hls_durations[v25+1];
                    ++v25;
                } while ( v25 != v24 );
            }
            else
            {
                p_this->p_structure->i_history_count = v5 + 1;
            }
            v6 = v3->i_bitrate;
            v7 = v3->i_size;
            float_t f_duration = v3->f_duration;//p_piece = (metadata_piece_t *)LODWORD(v3->f_duration);
            freeSegment(v3);
            //v9 = this->p_this->p_structure;
            //v10 = v9;
            //v30 = this->p_this->p_structure;
            v11 = p_this->p_structure->i_history_count;
            p_this->p_structure->f_hls_bitrates[v11-1] = v6;
            p_this->p_structure->f_hls_lengths[v11-1] = v7;
            p_this->p_structure->f_hls_durations[v11-1] = (signed int)(f_duration * 1000000.0);
            if ( v11 <= 0 )
            {
                v17 = 0.0;
                v18 = 0.0;
                v19 = 0.0;
            }
            else
            {
                //v12 = v30;
                v13 = 0;
                v14 = 0;
                v15 = 0;
                v16 = 0;
                do
                {
                    v16 += p_this->p_structure->f_hls_bitrates[v13];
                    v15 += p_this->p_structure->f_hls_lengths[v13];
                    v14 += p_this->p_structure->f_hls_durations[v13];
                    ++v13;
                } while ( v13 != v11 );
                v17 = (long double)v16;
                v18 = (long double)v15;
                v19 = (long double)v14;
            }
            v20 = (long double)v11;
            v21 = v20;
            v22 = v17 / v20;
            v23 = p_this->p_structure->b_p2p_enabled == 0;
            p_this->p_structure->f_avg_hls_bitrate = v22;
            p_this->p_structure->f_avg_hls_length = v18 / v21;
            p_this->p_structure->f_avg_usec_x_segment = v19 / v21;
            p_this->p_structure->f_avg_segment_x_sec = v22 / (v18 / v21);
            if ( !v23 )
            {
                v26 = 0;
                v27 = p_this->p_structure->p_metadataManager->getSegmentPieces(i_quality, i_segment_id);
                if ( v27 )
                {
                    while ( v26 < array_count(v27) )
                    {
                        v28 = array_item_at_index(v27, v26);
                        if ( v28 )
                        {
                            UpdateP2PStats(i_quality, i_segment_id, v28->i_piece_id);
                            freePiece(v28);
                        }
                        ++v26;
                    }
                    array_destroy(v27);
                }
            }
            if ( i_segment_id == 0xA * (i_segment_id / 0xA) )
            {
                print_msg(__func__, "**********************************************************");
                print_msg(__func__, "Average values: ");
                print_msg(__func__,
                          " - f_avg_hls_bitrate      = %f (bits per second)",
                          this->p_this->p_structure->f_avg_hls_bitrate);
                print_msg(__func__, " - f_avg_hls_length       = %f (bytes)", this->p_this->p_structure->f_avg_hls_length);
                print_msg(__func__,                        " - f_avg_usec_x_segment   = %f (microseconds)",
                          this->p_this->p_structure->f_avg_usec_x_segment);
                print_msg(__func__,                        " - f_avg_segment_x_sec    = %f (segments per seconds)",
                          this->p_this->p_structure->f_avg_segment_x_sec);
                print_msg(__func__, " --------------");
                print_msg(__func__,
                          " - f_avg_p2p_bitrate      = %f (bits per second)",
                          this->p_this->p_structure->f_avg_p2p_bitrate);
                print_msg(__func__, " - f_avg_p2p_length       = %f (bytes)", this->p_this->p_structure->f_avg_p2p_length);
                print_msg(__func__,                        " - f_avg_usec_x_piece     = %f (microseconds)",
                          this->p_this->p_structure->f_avg_usec_x_piece);
                print_msg(__func__,
                          " - f_avg_piece_x_sec      = %f (pieces per seconds)",
                          this->p_this->p_structure->f_avg_piece_x_sec);
                print_msg(__func__, "**********************************************************");
            }
        }
    }
}

int btContent::GetOptimisticDownInterval() {
    int v1; // eax@2
    int result; // eax@2

    if ( this->p_this->p_structure->WORLD->GetDownloads() <= 3 )
    {
        result = random() % 3 + 2;
    }
    else
    {
        v1 = random();
        result = ((((unsigned int)(v1 >> 0x1F) >> 0x1E) + (_BYTE)v1) & 3) - ((unsigned int)(v1 >> 0x1F) >> 0x1E) + 6;
    }
    return result;
}

block_t *btContent::ReadSegment(size_t i_quality, size_t i_segment_id) {
    if ( this->p_cache && this->p_hls_bitfield->IsSet(i_quality, i_segment_id) )
        return this->p_cache->ReadSegment(i_quality, i_segment_id);
    return 0;
}

block_t *btContent::ReadPiece(size_t i_piece_quality, size_t i_piece_id) {
    if ( this->p_cache )
        return this->p_cache->GetPiece(i_piece_quality, i_piece_id);
    return 0;
}

array_t<metadata_keyframe_t> *MetadataManager::getSegmentKeyframes(const int i_current_stream, const int i_segment) {
    if ( this->p_metadata_content )
        return this->p_metadata_content->getSegmentKeyframes(i_current_stream, i_segment);
    return 0;
}

int MetadataManager::getPieceSegment(const int i_piece_id) {
    //int result; // eax@2

    if ( this->p_metadata_content )
        return this->p_metadata_content->getPieceSegment(i_piece_id);
    return 0x80000001;
}

int
MetadataManager::segmentID2PieceID(const int i_segment_id, const int i_stream, const int i_criteria,
                                   const int i_order) {
    int i_dummy; // [sp+2Ch] [bp-10h]@2

    if ( this->p_metadata_content )
        return this->p_metadata_content->segmentID2PieceID(i_segment_id, i_stream, i_criteria, i_order, &i_dummy);
    return 0x80000001;
}

void * run_metadata_content(void* p_data)
{
    MetadataContent *v2; // esi@5
    char value; // [sp+1Ch] [bp-10h]@8
    long* p_longdata = (long*)p_data;
    MetadataManager* manager = (MetadataManager*)p_longdata[1];
    goalbit_t_0* goalbit_t_01 = (goalbit_t_0*)p_longdata[0];
    if ( p_data && manager && goalbit_t_01 )
    {
        v2 = new MetadataContent(goalbit_t_01, manager);
        if ( v2 )
        {
            manager->p_metadata_content = v2;
            v2->Run();
            manager->b_ready = false;
        }
        manager->b_ready = false;
        free(p_data);
        pthread_exit(&value);
    }
    return 0;
}

MetadataManager::MetadataManager(goalbit_t_0 *p_goalbit) {
    long * v4; // eax@1
    //goalbit_t_0 *v5; // edx@1

    b_ready = 0;
    p_this = p_goalbit;
    p_metadata_content = 0;
    pthread_mutex_init(&content_lock, 0);
    this->p_metadata_thread = new pthread_t();
    v4 = (long*)malloc(sizeof(void*)* 2);
    //v5 = this->p_this;
    v4[1] = (long)this;
    v4[0] = (long)p_this;
    if ( pthread_create(this->p_metadata_thread, 0, run_metadata_content, (void *)v4) )
    {
        print_err(__func__, "[MetadataManager] Can't create MetadataContent thread!!");
        p_metadata_content = 0;
    }
    return;
}

bool MetadataManager::isReady() {
    return this->b_ready;
}

char *MetadataManager::getSegmentMD5(const int i_current_stream, const int i_segment) {
    if ( p_metadata_content )
        return p_metadata_content->getSegmentMD5(i_current_stream, i_segment);
    return 0;
}

char *MetadataManager::getPieceMD5(const int i_current_stream, const int i_segment,
                                   const int i_piece_id) {
    char *result; // eax@2

    if ( this->p_metadata_content )
        result = this->p_metadata_content->getPieceMD5(i_current_stream, i_segment, i_piece_id);
    else
        result = 0;
    return result;
}

metadata_piece_t *
MetadataManager::getPiece(const int i_current_stream, const int i_segment, const int i_piece) {
    metadata_piece_t *result; // eax@2

    if ( this->p_metadata_content )
        result = this->p_metadata_content->getSegmentPiece(i_current_stream, i_segment, i_piece);
    else
        result = 0;
    return result;
}

void MetadataManager::setReady(bool ready) {
    this->b_ready = ready;
}

uint64_t MetadataManager::getSegmentSize(const int i_current_stream, const int i_segment) {
    uint64_t result; // rax@2

    if ( this->p_metadata_content )
        result = this->p_metadata_content->getSegmentSize(i_current_stream, i_segment);
    else
        result = 0LL;
    return result;
}

int MetadataManager::getBiggestPieceID(const int i_stream) {
    MetadataContent *v2; // eax@1
    int v3; // edx@2
    int result; // eax@2

    v2 = this->p_metadata_content;
    if ( v2 )
    {
        v3 = v2->getLastSegment(i_stream);
        result = 0x80000001;
        if ( v3 != 0x80000001 )
            result = segmentID2PieceID(v3, i_stream, 3, 2);
    }
    else
    {
        result = 0x80000001;
    }
    return result;
}

uint64_t MetadataManager::getPieceSize(const int i_current_stream, const int i_segment,
                                       const int i_piece_id) {
    if ( this->p_metadata_content )
        return this->p_metadata_content->getPieceSize(i_current_stream, i_segment, i_piece_id);
    return 0;
}

array_t<metadata_piece_t> *MetadataManager::getSegmentPieces(const int i_current_stream, const int i_segment)
{
    if ( p_metadata_content )
        return p_metadata_content->getSegmentPieces(i_current_stream, i_segment);
    return 0;
}

MetadataManager::~MetadataManager() {
    this->b_ready = false;
    pthread_mutex_destroy(&this->content_lock);
    if ( this->p_metadata_thread )
    {
        pthread_join(*this->p_metadata_thread, 0);
        free(this->p_metadata_thread);
        this->p_metadata_thread = 0;
    }
    if ( this->p_metadata_content )
    {
        delete  this->p_metadata_content;
    }
}

metadata_stream_t* getStream(const metadata_content_t *p_content, int wanted)
{
    int v2; // eax@2
    //metadata_stream_t *result; // eax@5

    if ( !p_content || (v2 = array_count(p_content->p_streams), v2 <= 0) || v2 <= wanted || (unsigned int)wanted >> 0x1F )
        return 0;//result = 0;
    return array_item_at_index(p_content->p_streams, wanted);
}

hls_stream_t* getStream(hls_content_t *p_content, int wanted)
{
    int v2; // eax@2
    hls_stream_t *result; // eax@5

    if ( !p_content || (v2 = array_count(p_content->p_streams), v2 <= 0) || v2 <= wanted || (unsigned int)wanted >> 31 )
        result = 0;
    else
        result = array_item_at_index(p_content->p_streams, wanted);
    return result;
}

void _assert_fail(const char* a1, const char* a2, int a3, const char* a4)
{
}

metadata_segment_t* getSegment(const metadata_stream_t *p_stream, const int index)
{
    int v2; // eax@2
    metadata_segment_t *result; // eax@5

    if ( !p_stream )
        _assert_fail(
                "p_stream",
                "src/basetools/metadata.cpp",
                457,
                "metadata_segment_t* getSegment(const metadata_stream_t*, int)");
    v2 = array_count(p_stream->p_segments);
    if ( v2 <= 0 || v2 <= index || (unsigned int)index >> 31 )
        result = 0;
    else
        result = array_item_at_index(p_stream->p_segments, index);
    return result;
}

metadata_segment_t* findSegment(const metadata_stream_t *p_stream, const int sequence)
{
    int v2; // edi@2
    metadata_segment_t *result; // eax@2
    int v4; // ebx@3

    if ( !p_stream )
        _assert_fail(
                "p_stream",
                "src/basetools/metadata.cpp",
                472,
                "metadata_segment_t* findSegment(const metadata_stream_t*, int)");
    v2 = array_count(p_stream->p_segments);
    result = 0;
    if ( v2 > 0 )
    {
        v4 = 0;
        while ( 1 )
        {
            result = getSegment(p_stream, v4);
            if ( result )
            {
                if ( result->i_sequence == sequence )
                    break;
            }
            if ( ++v4 == v2 )
                return 0;
        }
    }
    return result;
}

template <typename T>
void array_init(array_t<T> *p_array)
{
    p_array->i_count = 0;
    p_array->pp_elems = 0;
}

template <typename T>
array_t<T>* array_new()
{
    array_t<T>* v0; // eax@1
    array_t<T>* v1; // ebx@1

    v0 = new array_t<T>();
    v1 = v0;
    if ( v0 )
        array_init(v0);
    return v1;
}

metadata_keyframe_t* getKeyframe(const metadata_segment_t *p_segment, const int index)
{
    int v2; // eax@2
    metadata_keyframe_t *result; // eax@5

    if ( !p_segment )
        _assert_fail(
                "p_segment",
                "src/basetools/metadata.cpp",
                834,
                "metadata_keyframe_t* getKeyframe(const metadata_segment_t*, int)");
    v2 = array_count(p_segment->p_keyframes);
    if ( v2 <= 0 || v2 <= index || (unsigned int)index >> 31 )
        result = 0;
    else
        result = array_item_at_index(p_segment->p_keyframes, index);
    return result;
}

metadata_keyframe_t* copyKeyframe(const metadata_keyframe_t *p_keyframe)
{
    metadata_keyframe_t *result; // eax@1

    result = 0;
    if ( p_keyframe )
    {
        result = new metadata_keyframe_t();
        if ( result )
            *result = *p_keyframe;
    }
    return result;
}

array_t<metadata_keyframe_t> *MetadataContent::getSegmentKeyframes(const int i_current_stream, const int i_segment) {
    metadata_segment_t *v6; // eax@5
    const metadata_segment_t *v7; // esi@5
    int v8; // ebx@7
    int i; // ST04_4@8
    metadata_keyframe_t *v10; // eax@8
    metadata_keyframe_t *v11; // eax@8
    //MetadataManager *v12; // eax@10

    //v3 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_lock(36, &p_parent->content_lock);
    array_t<metadata_keyframe_t>* v4 = 0;
    if ( this->p_content )
    {
        const metadata_stream_t *v5 = getStream(this->p_content, i_current_stream);
        if ( v5 )
        {
            v6 = findSegment(v5, i_segment);
            v7 = v6;
            if ( v6 )
            {
                if ( v6->p_keyframes )
                {
                    v8 = 0;
                    v4 = array_new<metadata_keyframe_t>();
                    while ( v8 < array_count(v7->p_keyframes) )
                    {
                        i = v8++;
                        v10 = getKeyframe(v7, i);
                        v11 = copyKeyframe(v10);
                        array_append(v4, v11);
                    }
                }
            }
        }
    }
    //v12 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_unlock(36, &p_parent->content_lock);
    return v4;
}

void * hash_get_value_for_key(const hash_t_0 *p_dict, const char *psz_key)
{
    int v2; // edx@2
    char v3; // al@3
    const char *v4; // esi@4
    //int v6; // ebp@4
    //__int64 v7; // rax@5
    //__int64 v8; // rcx@5
    //nt v9; // edi@5
    hash_entry_t_0 *v10; // esi@7
    void *result; // eax@10

    if ( !p_dict->p_entries )
        goto LABEL_10;
    v2 = 0;
    if ( psz_key )
    {
        v3 = *psz_key;
        if ( *psz_key )
        {
            v4 = psz_key;
            unsigned long long v6 = 0;
            unsigned int v5 = 0;
            do
            {
                v6 = (v5+*v4)*0x401;
                v5 = (v6 >> 8) ^ v6;
                ++v4;
            } while ( *v4 );
            v2 = v5 % p_dict->i_size;//v2 = __PAIR__((unsigned int)v6, v5) % p_dict->i_size;
        }
    }
    v10 = p_dict->p_entries[v2];
    if ( v10 )
    {
        while ( strcmp(psz_key, v10->psz_key) )
        {
            v10 = v10->p_next;
            if ( !v10 )
                goto LABEL_10;
        }
        result = v10->p_value;
    }
    else
    {
        LABEL_10:
        result = 0;
    }
    return result;
}

unsigned int MetadataContent::getPieceSegment(int i_piece_id) {
    //MetadataManager *v2; // eax@1
    unsigned int v3; // esi@5
    //MetadataManager *v4; // eax@7
    char *psz_key; // [sp+2Ch] [bp-10h]@1

    psz_key = 0;
    //v2 = this->p_parent;
    if ( p_parent ) {
        Mypthread_mutex_lock(37, &p_parent->content_lock);
    }
    if ( this->p_content && this->p_pieces )
    {
        v3 = 0x80000001;
        asprintf(&psz_key, "%s%d", "piece_", i_piece_id);
        if ( hash_contains_key(this->p_pieces, psz_key) )
        {
            v3 = *(int*)hash_get_value_for_key(this->p_pieces, psz_key);
            free(psz_key);
        }
    }
    else
    {
        v3 = 0x80000001;
    }
    //v4 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_unlock(37, &p_parent->content_lock);
    return v3;
}

metadata_piece_t* getPiece(const metadata_segment_t *p_segment, const int index)
{
    int v2; // eax@2
    metadata_piece_t *result; // eax@5

    if ( !p_segment )
        _assert_fail(
                "p_segment",
                "src/basetools/metadata.cpp",
                0x2BF,
                "metadata_piece_t* getPiece(const metadata_segment_t*, int)");
    v2 = array_count(p_segment->p_pieces);
    if ( v2 <= 0 || v2 <= index || (unsigned int)index >> 0x1F )
        result = 0;
    else
        result = array_item_at_index(p_segment->p_pieces, index);
    return result;
}

metadata_segment_t* searchSegmentWithPieces(const metadata_stream_t *p_stream, const int i_sequence, const int i_orderby)
{
    int v3; // ebp@2
    int v4; // ebx@3
    metadata_segment_t *v6; // esi@6
    array_t<metadata_piece_t> *v7; // eax@7
    int v8; // eax@9
    int i_candidate_id; // [sp+18h] [bp-24h]@3
    int i_candidate_pos; // [sp+1Ch] [bp-20h]@3

    if ( !p_stream )
        _assert_fail(
                "p_stream",
                "src/basetools/metadata.cpp",
                0x200,
                "metadata_segment_t* searchSegmentWithPieces(const metadata_stream_t*, int, int)");
    v3 = array_count(p_stream->p_segments);
    if ( v3 <= 0 )
        return 0;
    v4 = 0;
    i_candidate_pos = 0x80000001;
    i_candidate_id = 0x80000001;
    do
    {
        v6 = getSegment(p_stream, v4);
        if (!v6) {
            //goto LABEL_5;
            v4++;
            continue;
        }
        v7 = v6->p_pieces;
        if (!v7 || !array_count(v7)) {
            //goto LABEL_5;
            v4++;
            continue;
        }
        v8 = v6->i_sequence;
        if (v8 == i_sequence)
            return v6;
        if (i_orderby == 2 && v8 > i_sequence) {
            if (i_candidate_id == 0x80000001 || v8 < i_candidate_id) {
                i_candidate_pos = v4;
                i_candidate_id = v6->i_sequence;
            }
        } else if (i_orderby == 3 && v8 < i_sequence) {
            if (i_candidate_id == 0x80000001 || v8 > i_candidate_id) {
                i_candidate_pos = v4;
                i_candidate_id = v6->i_sequence;
            }
        }
        ++v4;
    } while ( v4 != v3 );
    if ( i_candidate_pos != 0x80000001 )
        return getSegment(p_stream, i_candidate_pos);
    return 0;
}

int
MetadataContent::segmentID2PieceID(const int i_segment_id, const int i_stream, const int i_criteria,
                                   const int i_order, int *i_selected_segment_id) {
    //MetadataManager *v6; // eax@2
    int v7; // ebx@4
    const metadata_stream_t *v8; // eax@9
    int v9; // edx@13
    int v10; // ebx@15
    metadata_piece_t *v11; // eax@17
    //MetadataManager *v12; // eax@19
    metadata_segment_t *v14; // eax@22
    int v15; // eax@33
    int v16; // eax@35
    //bool v17; // cl@36
    metadata_segment_t *p_candidate_segment; // [sp+2Ch] [bp-30h]@4
    int i_bigger_segment_id; // [sp+38h] [bp-24h]@34
    int i_smaller_segment_id; // [sp+3Ch] [bp-20h]@34

    if ( i_criteria == 4 )
    {
        i_bigger_segment_id = 0;
        i_smaller_segment_id = 0;
        v10 = segmentID2PieceID(i_segment_id, i_stream, 2, i_order, &i_bigger_segment_id);
        if ( i_bigger_segment_id == i_segment_id )
            return v10;
        v16 = segmentID2PieceID(i_segment_id, i_stream, 3, i_order, &i_smaller_segment_id);
        if ( i_smaller_segment_id == i_segment_id )
            return v16;
        //v17 = v10 == 0x80000001;
        if ( v16 != 0x80000001 )
        {
            if ( v10 != 0x80000001 )
            {
                if ( (((i_segment_id - i_bigger_segment_id) >> 0x1F) ^ (i_segment_id - i_bigger_segment_id))
                     - ((i_segment_id - i_bigger_segment_id) >> 0x1F) > (((i_segment_id - i_smaller_segment_id) >> 0x1F) ^ (i_segment_id - i_smaller_segment_id))
                                                                        - ((i_segment_id - i_smaller_segment_id) >> 0x1F) )
                    v10 = v16;
                return v10;
            }
            return v16;
        }
        if ( v10 == 0x80000001 )
            v10 = 0x80000001;
        return v10;
    }
    //v6 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_lock(38, &p_parent->content_lock);
    v7 = 0;
    p_candidate_segment = 0;
    while ( v7 < array_count(this->p_content->p_streams) )
    {
        if ( i_stream == v7 || i_stream == 0x80000001 )
        {
            v8 = getStream(this->p_content, v7);
            if ( v8 ) {
                //break;
                if ( i_criteria == 2 )
                {
                    v14 = searchSegmentWithPieces(v8, i_segment_id, 2);
                    if ( !v14 ) {
                        //goto LABEL_6;
                        v7++;
                        continue;
                    }
                    v9 = v14->i_sequence;
                    if ( p_candidate_segment ) {
                        //goto LABEL_28;
                        v9 = p_candidate_segment->i_sequence;
                        if (v14->i_sequence < p_candidate_segment->i_sequence) {
                            v9 = v14->i_sequence;
                            p_candidate_segment = v14;
                        }
                    } else {
                        p_candidate_segment = v14;
                    }
                }
                else
                {
                    if ( i_criteria == 3 )
                    {
                        v14 = searchSegmentWithPieces(v8, i_segment_id, 3);
                        if ( !v14 ) {
                            //goto LABEL_6;
                            v7++;
                            continue;
                        }
                        if ( !p_candidate_segment )
                        {
                            //LABEL_23:
                            v9 = v14->i_sequence;
                            p_candidate_segment = v14;
                            //goto LABEL_14;
                        } else {
                            v9 = p_candidate_segment->i_sequence;
                            if (v14->i_sequence > p_candidate_segment->i_sequence) {
                                v9 = v14->i_sequence;
                                //LABEL_28:
                                p_candidate_segment = v14;
                                //goto LABEL_14;
                            }
                        }
                    } else {
                        if (i_criteria == 1) {
                            v14 = searchSegmentWithPieces(v8, i_segment_id, 1);
                            if (!v14) {
                                //goto LABEL_6;
                                v7++;
                                continue;
                            }
                            //goto LABEL_23;
                            v9 = v14->i_sequence;
                            p_candidate_segment = v14;
                        } else {
                            v9 = p_candidate_segment->i_sequence;
                        }
                        //goto LABEL_14;
                    }
                }
                //LABEL_14:
                if ( i_segment_id == v9 ) {
                    break;//goto LABEL_15;
                }
            }
        }
        //LABEL_6:
        ++v7;
    }
    //LABEL_15:
    v10 = 0x80000001;
    if ( p_candidate_segment )
    {
        *i_selected_segment_id = p_candidate_segment->i_sequence;
        if ( i_order == 2 )
        {
            v15 = array_count(p_candidate_segment->p_pieces);
            v11 = getPiece(p_candidate_segment, v15 - 1);
        }
        else
        {
            v11 = getPiece(p_candidate_segment, 0);
        }
        v10 = v11->i_piece_id;
    }
    //v12 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_unlock(38, &p_parent->content_lock);
    return v10;
}

char *MetadataContent::getSegmentMD5(const int i_current_stream, const int i_segment) {
    //MetadataManager *v3; // eax@1
    char* v4; // esi@3
    const metadata_stream_t *v5; // eax@4
    metadata_segment_t *v6; // eax@5
    //MetadataManager *v7; // eax@7

    //v3 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_lock(39, &p_parent->content_lock);
    v4 = 0;
    if ( this->p_content )
    {
        v5 = getStream(this->p_content, i_current_stream);
        if ( v5 )
        {
            v6 = findSegment(v5, i_segment);
            if ( v6 )
                v4 = strdup(v6->psz_MD5);
        }
    }
    //v7 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_unlock(39, &p_parent->content_lock);
    return (char *)v4;
}

metadata_piece_t* findPiece(const metadata_segment_t *p_segment, const int piece_id)
{
    int v2; // edi@2
    metadata_piece_t *result; // eax@2
    int v4; // ebx@3

    if ( !p_segment )
        _assert_fail(
                "p_segment",
                "src/basetools/metadata.cpp",
                0x2CE,
                "metadata_piece_t* findPiece(const metadata_segment_t*, int)");
    v2 = array_count(p_segment->p_pieces);
    result = 0;
    if ( v2 > 0 )
    {
        v4 = 0;
        while ( 1 )
        {
            result = getPiece(p_segment, v4);
            if ( result )
            {
                if ( result->i_piece_id == piece_id )
                    break;
            }
            if ( ++v4 == v2 )
                return 0;
        }
    }
    return result;
}

char *MetadataContent::getPieceMD5(const int i_current_stream, const int i_segment,
                                   const int i_piece_id) {
    //MetadataManager *v4; // eax@1
    char* v5; // esi@3
    const metadata_stream_t *v6; // eax@4
    const metadata_segment_t *v7; // eax@5
    metadata_piece_t *v8; // eax@6
    //MetadataManager *v9; // eax@8

    //v4 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_lock(40, &p_parent->content_lock);
    v5 = 0;
    if ( this->p_content )
    {
        v6 = getStream(this->p_content, i_current_stream);
        if ( v6 )
        {
            v7 = findSegment(v6, i_segment);
            if ( v7 )
            {
                v8 = findPiece(v7, i_piece_id);
                if ( v8 )
                    v5 = strdup(v8->psz_MD5);
            }
        }
    }
    //v9 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_unlock(40, &p_parent->content_lock);
    return (char *)v5;
}

metadata_piece_t *MetadataContent::getSegmentPiece(const int i_current_stream, const int i_segment,
                                                   const int i_piece) {
    //MetadataManager *v4; // eax@1
    metadata_piece_t *v5; // edi@3
    const metadata_stream_t *v6; // eax@4
    metadata_segment_t *v7; // eax@5
    const metadata_segment_t *v8; // esi@5
    array_t<metadata_piece_t> *v9; // eax@6
    //MetadataManager *v10; // eax@9

    //v4 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_lock(41, &p_parent->content_lock);
    v5 = 0;
    if ( this->p_content )
    {
        v6 = getStream(this->p_content, i_current_stream);
        if ( v6 )
        {
            v7 = findSegment(v6, i_segment);
            v8 = v7;
            if ( v7 )
            {
                v9 = v7->p_pieces;
                if ( v9 && array_count(v9) > 0 )
                    v5 = findPiece(v8, i_piece);
                else
                    v5 = 0;
            }
        }
    }
    //v10 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_unlock(41, &p_parent->content_lock);
    return v5;
}

void MetadataContent::Run() {
    long double v1; // fst7@3
    metadata_content_t *v3; // eax@7

    while ( this->p_this->p_structure->b_btc_alive )
    {
        if ( mdate() - this->t_next_metadata_update > 0 )
        {
            v3 = this->p_content;
            if ( v3 )
                v1 = updateMetadata(v3->psz_metadata_url);
            else
                v1 = loadMetadata(this->p_this->p_structure->psz_p2p_metadata_url);
            this->t_next_metadata_update = (1000000.0 * v1) + mdate();
            print_dbg(__func__, "[MetadadtaContent] next P2P manifest reload in: %.02f secs", v1);
        }
        msleep(50000);
    }
}

metadata_content_t* newContent(const char *psz_url, const int i_program_id)
{
    metadata_content_t *v2; // ebx@1
    char* v3; // eax@5

    v2 = 0;
    if ( psz_url )
    {
        if ( *psz_url )
        {
            if ( i_program_id > 0 )
            {
                v2 = new metadata_content_t();
                if ( v2 )
                {
                    v3 = strdup(psz_url);
                    v2->i_program_id = i_program_id;
                    v2->psz_metadata_url = v3;
                    v2->p_streams = array_new<metadata_stream_t>();
                }
            }
        }
    }
    return v2;
}

metadata_segment_t * copySegment(const metadata_segment_t *p_segment)
{
    metadata_segment_t *v1; // edi@1
    metadata_segment_t *v2; // eax@2
    int v3; // esi@3
    //int v4; // edx@3
    //int v5; // eax@3
    int i; // ST04_4@4
    const metadata_piece_t *v7; // eax@4
    metadata_piece_t *v8; // eax@4
    int v9; // esi@6
    int v10; // ST04_4@7
    const metadata_keyframe_t *v11; // eax@7
    metadata_keyframe_t *v12; // eax@7

    v1 = 0;
    if ( p_segment )
    {
        v2 = new metadata_segment_t();
        v1 = v2;
        if ( v2 )
        {
            v3 = 0;
            //v4 = HIDWORD(p_segment->i_size);
            v2->i_sequence = p_segment->i_sequence;
            v2->i_duration = p_segment->i_duration;
            //v5 = p_segment->i_size;
            //HIDWORD(v1->i_size) = v4;
            //LODWORD(v1->i_size) = v5;
            v1->i_size = p_segment->i_size;
            v1->b_discontinuity = p_segment->b_discontinuity;
            v1->psz_MD5 = strdup(p_segment->psz_MD5);
            v1->p_pieces = array_new<metadata_piece_t>();
            v1->p_keyframes = array_new<metadata_keyframe_t>();
            while ( v3 < array_count(p_segment->p_pieces) )
            {
                i = v3++;
                v7 = getPiece(p_segment, i);
                v8 = copyPiece(v7);
                array_append(v1->p_pieces, v8);
            }
            v9 = 0;
            while ( v9 < array_count(p_segment->p_keyframes) )
            {
                v10 = v9++;
                v11 = getKeyframe(p_segment, v10);
                v12 = copyKeyframe(v11);
                array_append(v1->p_keyframes, v12);
            }
        }
    }
    return v1;
}

metadata_stream_t* copyStream(const metadata_stream_t *p_stream)
{
    metadata_stream_t *v1; // edi@1
    int v2; // ebx@3
    char* v3; // eax@3
    //int v4; // edx@3
    //int v5; // eax@3
    int i; // ST04_4@4
    const metadata_segment_t *v7; // eax@4
    metadata_segment_t *v8; // eax@4

    v1 = 0;
    if ( p_stream )
    {
        v1 = new metadata_stream_t();
        if ( v1 )
        {
            v2 = 0;
            v3 = strdup(p_stream->psz_name);
            //v4 = HIDWORD(p_stream->i_bitrate);
            v1->psz_name = (char *)v3;
            //v5 = p_stream->i_bitrate;
            //HIDWORD(v1->i_bitrate) = v4;
            //LODWORD(v1->i_bitrate) = v5;
            v1->i_bitrate = p_stream->i_bitrate;
            v1->i_video_width = p_stream->i_video_width;
            v1->i_video_height = p_stream->i_video_height;
            v1->p_segments = array_new<metadata_segment_t>();
            while ( v2 < array_count(p_stream->p_segments) )
            {
                i = v2++;
                v7 = getSegment(p_stream, i);
                v8 = copySegment(v7);
                array_append(v1->p_segments, v8);
            }
        }
    }
    return v1;
}

metadata_content_t* copyContent(const metadata_content_t *p_content)
{
    metadata_content_t *v1; // esi@1
    int v2; // ebx@3
    array_t<metadata_stream_t> *v3; // eax@3
    int i; // ST04_4@4
    const metadata_stream_t *v5; // eax@4
    metadata_stream_t *v6; // eax@4

    v1 = 0;
    if ( p_content )
    {
        v1 = new metadata_content_t();
        if ( v1 )
        {
            v2 = 0;
            v1->psz_metadata_url = (char *)strdup(p_content->psz_metadata_url);
            v1->i_program_id = p_content->i_program_id;
            v3 = array_new<metadata_stream_t>();
            v1->p_streams = v3;
            while ( v2 < array_count(v3) )
            {
                i = v2++;
                v5 = getStream(p_content, i);
                v6 = copyStream(v5);
                array_append(v1->p_streams, v6);
                v3 = v1->p_streams;
            }
        }
    }
    return v1;
}

metadata_stream_t * newStream(const char *name, const uint64_t bw, const int width, const int height)
{
    metadata_stream_t *v4; // ebx@1
    char* v5; // eax@4

    v4 = 0;
    if ( name )
    {
        if ( *name )
        {
            v4 = new metadata_stream_t();
            if ( v4 )
            {
                v5 = strdup(name);
                v4->i_bitrate = bw;
                v4->psz_name = v5;
                v4->i_video_width = width;
                v4->i_video_height = height;
                v4->p_segments = array_new<metadata_segment_t>();
            }
        }
    }
    return v4;
}

hls_stream_t *copyStream(const hls_stream_t *p_stream)
{
    hls_stream_t *v1; // edi@1
    hls_stream_t *v2; // eax@2
    int v3; // ebx@3
    int i; // ST04_4@4
    const hls_segment_t *v5; // eax@4
    hls_segment_t *v6; // eax@4

    v1 = 0;
    if ( p_stream )
    {
        v2 = new hls_stream_t();
        v1 = v2;
        if ( v2 )
        {
            v3 = 0;
            mapStream(p_stream, v2);
            v1->p_segments = array_new<hls_segment_t>();
            while ( v3 < array_count(p_stream->p_segments) )
            {
                i = v3++;
                v5 = getSegment(p_stream, i);
                v6 = copySegment(v5);
                array_append(v1->p_segments, v6);
            }
            pthread_mutex_init(&v1->lock, 0);
        }
    }
    return v1;
}

metadata_stream_t * mergeStreams(const metadata_stream_t *p_old_stream, const metadata_stream_t *p_new_stream, const size_t i_min_valid_sequence)
{
    char *v3; // ebx@3
    int i; // ebx@5
    const metadata_segment_t *v5; // eax@8
    metadata_segment_t *v6; // eax@10
    int j; // ebx@12
    metadata_segment_t *v9; // eax@15
    const metadata_segment_t *v10; // ebp@15
    metadata_segment_t *v11; // eax@18
    metadata_stream_t *p_stream; // [sp+2Ch] [bp-20h]@3

    if ( !p_old_stream )
    {
        p_old_stream = p_new_stream;
        return copyStream(p_old_stream);
    }
    if ( !p_new_stream )
        return copyStream(p_old_stream);
    v3 = p_new_stream->psz_name;
    p_stream = 0;
    if ( !strcmp(p_old_stream->psz_name, p_new_stream->psz_name) )
    {
        p_stream = newStream(v3, p_new_stream->i_bitrate, p_new_stream->i_video_width, p_new_stream->i_video_height);
        if ( p_stream )
        {
            for ( i = 0; i < array_count(p_old_stream->p_segments); ++i )
            {
                v5 = getSegment(p_old_stream, i);
                if ( v5 )
                {
                    if ( v5->i_sequence >= i_min_valid_sequence )
                    {
                        v6 = copySegment(v5);
                        array_append(p_stream->p_segments, v6);
                    }
                }
            }
            for ( j = 0; j < array_count(p_new_stream->p_segments); ++j )
            {
                v9 = getSegment(p_new_stream, j);
                v10 = v9;
                if ( v9 && !findSegment(p_old_stream, v9->i_sequence) && i_min_valid_sequence <= v10->i_sequence )
                {
                    v11 = copySegment(v10);
                    array_append(p_stream->p_segments, v11);
                }
            }
        }
    }
    return p_stream;
}

metadata_content_t* mergeContents(const metadata_content_t *p_old_content, const metadata_content_t *p_new_content, const size_t i_min_valid_sequence)
{
    //char *v3; // ebx@3
    int i; // ebx@6
    const metadata_stream_t *v5; // edi@7
    const metadata_stream_t *v6; // eax@8
    metadata_stream_t *v7; // eax@8
    metadata_content_t *p_content; // [sp+18h] [bp-24h]@3

    if ( !p_old_content )
    {
        p_old_content = p_new_content;
        return copyContent(p_old_content);
    }
    if ( !p_new_content )
        return copyContent(p_old_content);
    //v3 = p_new_content->psz_metadata_url;
    p_content = 0;
    if ( !strcmp(p_old_content->psz_metadata_url, p_new_content->psz_metadata_url)
         && p_old_content->i_program_id == p_new_content->i_program_id )
    {
        p_content = newContent(p_new_content->psz_metadata_url, p_old_content->i_program_id);
        if ( p_content )
        {
            for ( i = 0; i < array_count(p_new_content->p_streams); ++i )
            {
                v5 = getStream(p_new_content, i);
                if ( v5 )
                {
                    v6 = getStream(p_old_content, i);
                    v7 = mergeStreams(v6, v5, i_min_valid_sequence);
                    array_append(p_content->p_streams, v7);
                }
            }
        }
    }
    return p_content;
}

void StartExecution(goalbit_t_0 *p_this)
{
    //goalbit_t_0 *v1; // ebx@1
    goalbit_event_t_0 *v2; // eax@2
    //goalbit_param_t_0 *v3; // edx@2
    //goalbit_event_t_0 *v4; // esi@2
    //void *(*v5)(goalbit_t_0 *, goalbit_event_t_0 *); // eax@2
    //unsigned int v7; // edi@5
    //goalbit_structure_t *v8; // edx@5
    //unsigned int v9; // esi@5
    size_t v10; // eax@5
    __int64 v11; // kr00_8@5

    //v1 = p_this;
    print_dbg(__func__,
              "[BTContent] WriteSegment - Start execution at %u (ABD: %0.4f)",
              p_this->p_structure->exec_info.i_segment_id,
              p_this->p_structure->exec_info.f_abd);
    //    if ( p_this->p_param->pf_event_callback )
    //    {
    //        v2 = new goalbit_event_t_0();
    //        v2->g_value.i_int = 0;
    //        ReportEvent(p_this, v2);//goalbit_event_callback
    //    }
    p_this->p_param->p_external_ref->SetBufferingState(false);
    //v7 = HIDWORD(v6);
    //v8 = v1->p_structure;
    //v9 = v6;
    v10 = p_this->p_structure->i_buff_num;
    v11 = mdate() - p_this->p_structure->i_last_buff_start;
    if ( v10 )
        p_this->p_structure->i_total_rebuff_dur += v11;
    else
        p_this->p_structure->i_startup_buff_dur = v11;
    //LODWORD(v8->i_last_buff_start) = 0;
    //HIDWORD(v8->i_last_buff_start) = 0;
    p_this->p_structure->i_last_buff_start = 0;
    p_this->p_structure->i_buff_num = v10 + 1;
}

void freeSegment(metadata_segment_t *p_segment)
{
    array_t<metadata_piece_t> *v1; // eax@2
    int v2; // esi@2
    metadata_piece_t *v3; // eax@4
    array_t<metadata_keyframe_t> *v4; // eax@9
    int v5; // esi@9
    metadata_keyframe_t *v6; // eax@11

    if ( p_segment )
    {
        v1 = p_segment->p_pieces;
        v2 = 0;
        if ( v1 )
        {
            while ( v2 < array_count(v1) )
            {
                v3 = getPiece(p_segment, v2);
                if ( v3 )
                    freePiece(v3);
                v1 = p_segment->p_pieces;
                ++v2;
            }
            array_destroy(p_segment->p_pieces);
        }
        v4 = p_segment->p_keyframes;
        v5 = 0;
        if ( v4 )
        {
            while ( v5 < array_count(v4) )
            {
                v6 = getKeyframe(p_segment, v5);
                if ( v6 )
                    freeKeyframe(v6);
                v4 = p_segment->p_keyframes;
                ++v5;
            }
            array_destroy(p_segment->p_keyframes);
        }
        if ( p_segment->psz_MD5 )
            free(p_segment->psz_MD5);
        free(p_segment);
    }
}

void freeStream(metadata_stream_t *p_stream)
{
    array_t<metadata_segment_t> *v1; // eax@2
    int v2; // ebx@2
    metadata_segment_t *v3; // eax@4

    if ( p_stream )
    {
        v1 = p_stream->p_segments;
        v2 = 0;
        if ( v1 )
        {
            while ( v2 < array_count(v1) )
            {
                v3 = getSegment(p_stream, v2);
                if ( v3 )
                    freeSegment(v3);
                v1 = p_stream->p_segments;
                ++v2;
            }
            array_destroy(p_stream->p_segments);
        }
        if ( p_stream->psz_name )
            free(p_stream->psz_name);
        free(p_stream);
    }
}

void freeContent(metadata_content_t *p_content)
{
    array_t<metadata_stream_t> *v1; // eax@2
    int v2; // ebx@2
    metadata_stream_t *v3; // eax@4

    if ( p_content )
    {
        v1 = p_content->p_streams;
        v2 = 0;
        if ( v1 )
        {
            while ( v2 < array_count(v1) )
            {
                v3 = getStream(p_content, v2);
                if ( v3 )
                    freeStream(v3);
                v1 = p_content->p_streams;
                ++v2;
            }
            array_destroy(p_content->p_streams);
            p_content->p_streams = 0;
        }
        if ( p_content->psz_metadata_url ) {
            free(p_content->psz_metadata_url);
            p_content->psz_metadata_url = 0;
        }
        free(p_content);
    }
}

void free_piece(void *p_piece)
{
    if ( p_piece )
        free(p_piece);
}

void hash_clear(hash_t_0 *p_dict, void (*pf_free)(void *), void *p_obj)
{
    hash_entry_t_0 **v3; // eax@1
    hash_entry_t_0 *v4; // ebx@4
    hash_entry_t_0 *v5; // esi@7
    int i; // [sp+1Ch] [bp-20h]@3

    v3 = p_dict->p_entries;
    if ( v3 )
    {
        if ( p_dict->i_size > 0 )
        {
            i = 0;
            do
            {
                v4 = v3[i];
                if ( v4 )
                {
                    while ( 1 )
                    {
                        v5 = v4->p_next;
                        if ( pf_free ) {
                            pf_free(v4->p_value);
                        }
                        free(v4->psz_key);
                        free(v4);
                        if ( !v5 )
                            break;
                        v4 = v5;
                    }
                    v3 = p_dict->p_entries;
                }
                ++i;
            }
            while ( p_dict->i_size > i );
        }
        free(v3);
        p_dict->p_entries = 0;
    }
    p_dict->i_size = 0;
}

void hash_destroy(hash_t_0 *p_dict, void (*pf_free)(void *), void *p_obj)
{
    if ( p_dict )
    {
        hash_clear(p_dict, pf_free, p_obj);
        free(p_dict);
    }
}

void hash_init(hash_t_0 *p_dict, int i_size)
{
    int v2; // ebx@1
    hash_entry_t_0 **v3; // eax@2

    v2 = i_size;
    p_dict->p_entries = 0;
    if ( i_size > 0 )
    {
        v3 = (hash_entry_t_0 **)calloc(i_size, sizeof(hash_entry_t_0 *));
        p_dict->p_entries = v3;
        if ( !v3 ) {
            v2 = 0;
        }
    }
    p_dict->i_size = v2;
}

hash_t_0 *hash_new()
{
    hash_t_0 *v0; // eax@1
    hash_t_0 *v1; // ebx@1

    v0 = new hash_t_0();
    v1 = v0;
    if ( v0 )
        hash_init(v0, 5);
    return v1;
}

void __hash_insert(hash_t_0 *p_dict, const char *psz_key, void *p_value, bool rebuild)
{
    unsigned int v4; // edi@3
    //int v5; // ebp@3
    char v6; // al@4
    const char *v7; // esi@5
    //__int64 v8; // rax@6
    //__int64 v9; // rcx@6
    //int v10; // edi@6
    hash_entry_t_0 *v11; // esi@7
    hash_entry_t_0 **v12; // eax@7
    hash_entry_t *v13; // eax@8
    signed int v14; // edx@9
    int v15; // ebx@14
    hash_entry_t_0 *i; // esi@15
    int v17; // [sp+18h] [bp-34h]@3
    hash_t new_dict; // [sp+28h] [bp-24h]@13

    if ( !p_dict->p_entries )
        hash_init(p_dict, 1);
    v4 = 0;
    unsigned long long v5 = 0;
    v17 = p_dict->i_size;
    if ( psz_key )
    {
        v6 = *psz_key;
        if ( *psz_key )
        {
            v7 = psz_key;
            do
            {
                v5 = (v4+*v7)*0x401;
                v4 = (v5 >> 8) ^ v5;
                ++v7;
            } while ( *v7 );
        }
    }
    v11 = new hash_entry_t_0();
    v11->psz_key = (char *)strdup(psz_key);
    v11->p_value = p_value;
    v12 = &p_dict->p_entries[v4 % v17];//v12 = &p_dict->p_entries[__PAIR__((unsigned int)v5, v4) % v17];
    v11->p_next = *v12;
    *v12 = v11;
    if ( rebuild )
    {
        v13 = v11->p_next;
        if ( v13 )
        {
            v14 = 1;
            do
            {
                v13 = v13->p_next;
                ++v14;
            }
            while ( v13 );
            if ( v14 > 3 )
            {
                hash_init(&new_dict, (3 * p_dict->i_size + 6) / 2);
                if ( p_dict->i_size > 0 )
                {
                    v15 = 0;
                    do
                    {
                        for ( i = p_dict->p_entries[v15]; i; i = i->p_next )
                            __hash_insert(&new_dict, i->psz_key, i->p_value, 0);
                        ++v15;
                    }
                    while ( p_dict->i_size > v15 );
                }
                hash_clear(p_dict, 0, 0);
                *p_dict = new_dict;
            }
        }
    }
}

void hash_insert(hash_t_0 *p_dict, const char *psz_key, void *p_value)
{
    __hash_insert(p_dict, psz_key, p_value, 1);
}

long double MetadataContent::updateMetadata(const char *psz_metadata_url) {
    metadata_content_t *v2; // ebx@1
    //goalbit_structure_t *v4; // eax@2
    //size_t v5; // edx@2
    int v6; // ebp@2
    //MetadataManager *v7; // eax@4
    //size_t v8; // edx@4
    //metadata_content_t *v9; // eax@6
    int v11; // eax@10
    metadata_stream_t *v12; // eax@10
    int v13; // eax@12
    metadata_segment_t *v14; // edx@12
    //goalbit_structure_t *v15; // eax@14
    //MetadataManager *v16; // eax@17
    int v18; // ebx@25
    metadata_segment_t *v19; // esi@25
    metadata_piece_t *v20; // eax@27
    int v21; // eax@28
    int *v22; // eax@28
    //MetadataManager *v23; // eax@32
    //goalbit_t_0 *v24; // eax@34
    //goalbit_structure_t *v25; // edx@34
    //bool v26; // zf@34
    size_t i_min_valid_sequence; // [sp+20h] [bp-3Ch]@4
    const metadata_stream_t *i_min_valid_sequencea; // [sp+20h] [bp-3Ch]@9
    const metadata_stream_t *i_min_valid_sequenceb; // [sp+20h] [bp-3Ch]@11
    metadata_segment_t *i; // [sp+24h] [bp-38h]@10
    int ia; // [sp+24h] [bp-38h]@23
    metadata_stream_t *p_stream; // [sp+28h] [bp-34h]@23
    float f_next_reload; // [sp+2Ch] [bp-30h]@1
    char *psz_key; // [sp+3Ch] [bp-20h]@28

    v2 = readMetadata(psz_metadata_url);
    f_next_reload = 10.0;
    if ( !v2 ) {
        return f_next_reload;
    }
    print_msg(__func__,
              "[MetadadtaContent] Updating P2P metadata!");
    Mypthread_mutex_lock(42, &this->p_this->p_structure->exec_lock);
    //v4 = this->p_this->p_structure;
    i_min_valid_sequence = p_this->p_structure->i_hls_win_offset;
    v6 = p_this->p_structure->i_target_quality;
    if ( i_min_valid_sequence == 0x80000001 )
        i_min_valid_sequence = 0;
    //i_min_valid_sequence = v5;
    Mypthread_mutex_unlock(42, &p_this->p_structure->exec_lock);
    //v7 = this->p_parent;
    //v8 = i_min_valid_sequence;
    if ( p_parent )
    {
        Mypthread_mutex_lock(43, &p_parent->content_lock);
        //v8 = i_min_valid_sequence;
    }
    //v9 = mergeContents(v3, v2, v8);
    this->p_content = mergeContents(this->p_content, v2, i_min_valid_sequence);
    if ( this->p_content ) {
        f_next_reload = 10.0;
        i_min_valid_sequencea = getStream(this->p_content, v6);
        if ( i_min_valid_sequencea
             && (array_count(i_min_valid_sequencea->p_segments))
             && (v11 = array_count(i_min_valid_sequencea->p_segments),
                i = getSegment(i_min_valid_sequencea, v11 - 1),
                (v12 = getStream(v2, v6)) != 0)
             && (i_min_valid_sequenceb = v12, array_count(v12->p_segments)) )
        {
            v13 = array_count(i_min_valid_sequenceb->p_segments);
            v14 = getSegment(i_min_valid_sequenceb, v13 - 1);
            f_next_reload = 1.0;
            if ( i->i_sequence != v14->i_sequence )
                f_next_reload = v14->i_duration;
            //v15 = this->p_this->p_structure;
            if ( p_this->p_structure->b_p2p_syncing ) {
                if ( p_this->p_structure->p_hlsManager->getMaxSegmentID(v6) <= v14->i_sequence ) {
                    print_dbg(__func__, "[MetadadtaContent] The P2P manifest syncing is finished");
                    //v24 = this->p_this;
                    //v25 = this->p_this->p_structure;
                    //v26 = p_this->p_structure->b_buffering == 0;
                    Mypthread_mutex_lock(34, &this->p_this->p_structure->exec_lock);
                    p_this->p_structure->b_p2p_syncing = false;
                    bool b_buffering = p_this->p_structure->b_buffering;
                    Mypthread_mutex_unlock(34, &this->p_this->p_structure->exec_lock);
                    if ( b_buffering == false ) {
                        StartExecution(p_this);
                    }
                }
                else
                {
                    print_dbg(__func__, "[MetadadtaContent] The HLS server is ahead of the P2P one, let's do a fast reload.");
                    f_next_reload = 1.0;
                }
            }
        }
        freeContent(v2);
        hash_destroy(this->p_pieces, free_piece, this->p_this);
        this->p_pieces = hash_new();
        ia = 0;
        p_stream = getStream(this->p_content, 0);
        while ( ia < array_count(p_stream->p_segments) )
        {
            v18 = 0;
            v19 = getSegment(p_stream, ia);
            if ( v19 )
            {
                while ( v18 < array_count(v19->p_pieces) )
                {
                    v20 = getPiece(v19, v18);
                    if ( v20 )
                    {
                        v21 = v20->i_piece_id;
                        psz_key = 0;
                        asprintf(&psz_key, "%s%d", "piece_", v21);
                        v22 = (int*)malloc(sizeof(int*));
                        *v22 = v19->i_sequence;
                        hash_insert(this->p_pieces, psz_key, v22);
                        free(psz_key);
                    }
                    ++v18;
                }
            }
            ++ia;
        }
        if ( this->p_parent )
            Mypthread_mutex_unlock(43, &this->p_parent->content_lock);
        //print();
    }
    else
    {
        freeContent(v2);
        //v23 = this->p_parent;
        f_next_reload = 10.0;
        if ( this->p_parent )
            Mypthread_mutex_unlock(43, &this->p_parent->content_lock);
    }
    return f_next_reload;
}

metadata_content_t *MetadataContent::readMetadata(const char *psz_metadata_url) {
    unsigned int v4; // eax@5
    //void *v5; // esp@5
    const char *v6; // eax@5
    metadata_content_t *v8; // ebx@5
    //int v9; // edx@6
    char *p_buffer; // [sp+1Ch] [bp-2Ch]@5
    block_t *p_; // [sp+28h] [bp-20h]@4
    //int v13; // [sp+2Ch] [bp-1Ch]@1

    //v13 = *MK_FP(__GS__, 0x14);
    if ( psz_metadata_url
         && *psz_metadata_url
         && this->p_parser
         )
    {
        p_ = sendHTTPRequest(this->p_this, psz_metadata_url, 0x19000, 0xA);
        if (p_) {
            p_->p_buffer[p_->i_buffer_index] = 0;
            v4 = p_->i_buffer_size;
            p_->i_buffer_index++;
            p_buffer = (char *) alloca(v4 + 0x10);
            v6 = (const char *) p_->p_buffer;
            //p_buffer = (char *)(((unsigned int)&p_buffer + 3) & 0xFFFFFFF0);
            strcpy(p_buffer, v6);
            p_buffer[p_->i_buffer_size] = 0;
            v8 = this->p_parser->parseMetadata(p_buffer, p_->i_buffer_size, psz_metadata_url);
            block_Release(p_);
            return v8;
        }
    }
    //v9 = *MK_FP(__GS__, 0x14) ^ v13;
    return 0;
}

void MetadataContent::print() {
    //MetadataManager *v1; // eax@1
    metadata_stream_t *v2; // ebp@5
    metadata_segment_t *v3; // eax@8
    const metadata_segment_t *v4; // ebx@8
    int v5; // esi@9
    metadata_piece_t *v6; // eax@10
    int i; // esi@14
    metadata_keyframe_t *v8; // eax@15
    //MetadataManager *v9; // eax@21
    int i_segment_iter; // [sp+38h] [bp-24h]@6
    int i_stream_iter; // [sp+3Ch] [bp-20h]@3

    print_dbg(__func__,
              "\n\n[MetadataContent] print -------------------------------------------------------------");
    //v1 = this->p_parent;
    if (this->p_parent)
        Mypthread_mutex_lock(44, & this->p_parent->content_lock);
    for (i_stream_iter = 0;
         i_stream_iter < array_count(this->p_content->p_streams); ++i_stream_iter) {
        v2 = getStream(this->p_content, i_stream_iter);
        if (v2) {
            print_dbg(__func__,
                      "Quality: %d ****************",
                      i_stream_iter);
            for (i_segment_iter = 0;
                 i_segment_iter < array_count(v2->p_segments); ++i_segment_iter) {
                v3 = getSegment(v2, i_segment_iter);
                v4 = v3;
                if (v3) {
                    v5 = 0;
                    print_dbg(__func__,
                              "\tSegment ID: %d, size: %lld B, duration: %f ms, MD5: %s",
                              v3->i_sequence,
                              v3->i_size,
                              (float_t )v3->i_duration,
                              v3->psz_MD5);
                    while (v5 < array_count(v4->p_pieces)) {
                        v6 = getPiece(v4, v5);
                        if (v6)
                            print_dbg(__func__,
                                      "\t\tPiece ID: %d, offset: %lld B, size: %lld B, duration: %f ms, MD5: %s",
                                      v6->i_piece_id,
                                      v6->i_offset,
                                      v6->i_size,
                                      (float_t )v6->i_duration,
                                      v6->psz_MD5);
                        ++v5;
                    }
                    for (i = 0; i < array_count(v4->p_keyframes); ++i) {
                        v8 = getKeyframe(v4, i);
                        if (v8)
                            print_dbg(__func__,
                                      "\t\t* Keyframe offset: %d, duration: %0.4f secs",
                                      v8->i_offset,
                                      v8->i_duration);
                    }
                }
            }
        }
    }
    //v9 = this->p_parent;
    if (p_parent)
        Mypthread_mutex_unlock(44, &p_parent->content_lock);
    print_dbg(__func__,
              "[MetadataContent] -------------------------------------------------------------------\n");
}

long double MetadataContent::loadMetadata(const char *psz_metadata_url) {
    int v7; // esi@10
    //metadata_stream_t *v8; // eax@10
    const metadata_stream_t *v9; // ebx@10
    int v10; // ebx@15
    metadata_segment_t *v11; // esi@15
    metadata_piece_t *v12; // eax@17
    int v13; // eax@18
    int *v14; // eax@18
    //MetadataManager *v15; // eax@23
    int v17; // eax@27
    //metadata_segment_t *v18; // eax@27
    metadata_segment_t *v19; // ebx@27
    //goalbit_structure_t *v20; // eax@27
    //goalbit_t_0 *v21; // eax@30
    //goalbit_structure_t *v22; // edx@30
    //bool v23; // zf@30
    int i; // [sp+24h] [bp-38h]@13
    metadata_stream_t *p_stream; // [sp+28h] [bp-34h]@13
    float f_next_reload; // [sp+2Ch] [bp-30h]@7
    char *psz_key; // [sp+3Ch] [bp-20h]@18

    //v2 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_lock(45, &p_parent->content_lock);
    if ( this->p_content )
    {
        freeContent(this->p_content);
    }
    this->p_content = readMetadata(psz_metadata_url);
    if ( p_parent )
    {
        Mypthread_mutex_unlock(45, &p_parent->content_lock);
        //v3 = this->p_content;
    }
    f_next_reload = 10.0;
    if ( this->p_content )
    {
        //v5 = this->p_parent;
        if ( p_parent ) {
            Mypthread_mutex_lock(46, &p_parent->content_lock);
        }
        Mypthread_mutex_lock(47, &this->p_this->p_structure->exec_lock);
        v7 = p_this->p_structure->i_target_quality;
        Mypthread_mutex_unlock(47, &p_this->p_structure->exec_lock);
        v9 = getStream(this->p_content, 0);
        //v9 = v8;
        if ( v9 && array_count(v9->p_segments) )
        {
            v17 = array_count(v9->p_segments);
            v19 = getSegment(v9, v17 - 1);
            //v19 = v18;
            f_next_reload = v19->i_duration;
            //v20 = this->p_this->p_structure;
            if ( p_this->p_structure->b_p2p_syncing )
            {
                if ( p_this->p_structure->p_hlsManager->getMaxSegmentID(v7) <= v19->i_sequence )
                {
                    print_dbg(__func__, "[MetadadtaContent] The P2P manifest syncing is finished");
                    //v21 = this->p_this;
                    //v22 = this->p_this->p_structure;
                    //v23 = p_this->p_structure->b_buffering == 0;
                    Mypthread_mutex_lock(47, &this->p_this->p_structure->exec_lock);
                    p_this->p_structure->b_p2p_syncing = false;
                    bool b_buffering = p_this->p_structure->b_buffering;
                    Mypthread_mutex_unlock(47, &this->p_this->p_structure->exec_lock);
                    if ( b_buffering == false ) {
                        StartExecution(p_this);
                    }
                }
                else
                {
                    print_dbg(__func__, "[MetadadtaContent] The HLS server is ahead of the P2P one, let's do a fast reload.");
                    f_next_reload = 1.0;
                }
            }
        }
        else
        {
            f_next_reload = 10.0;
        }
        i = 0;
        p_stream = getStream(this->p_content, v7);
        while ( i < array_count(p_stream->p_segments) )
        {
            v10 = 0;
            v11 = getSegment(p_stream, i);
            if ( v11 )
            {
                while ( v10 < array_count(v11->p_pieces) )
                {
                    v12 = getPiece(v11, v10);
                    if ( v12 )
                    {
                        v13 = v12->i_piece_id;
                        psz_key = 0;
                        asprintf(&psz_key, "%s%d", "piece_", v13);
                        v14 = (int*)malloc(sizeof(int*));
                        *v14 = v11->i_sequence;
                        hash_insert(this->p_pieces, psz_key, v14);
                        free(psz_key);
                    }
                    ++v10;
                }
            }
            ++i;
        }
        if ( this->p_parent )
        {
            this->p_parent->setReady(true);
            //v15 = this->p_parent;
            Mypthread_mutex_unlock(46, &p_parent->content_lock);
        }
        //print();
    } else {
        print_err(__func__, "loadMetadata failed.");
    }
    return f_next_reload;
}

int MetadataContent::getSegmentSize(const int i_current_stream, const int i_segment) {
    //MetadataManager *v3; // eax@1
    int v4; // esi@3
    const metadata_stream_t *v5; // eax@4
    metadata_segment_t *v6; // eax@5
    //int v7; // edi@6
    //MetadataManager *v8; // eax@7

    //v3 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_lock(48, &p_parent->content_lock);
    v4 = 0;
    if ( this->p_content )
    {
        v5 = getStream(this->p_content, i_current_stream);
        if ( v5 )
        {
            v6 = findSegment(v5, i_segment);
            if ( v6 )
            {
                v4 = v6->i_size;
                //v7 = HIDWORD(v6->i_size);
            }
        }
    }
    //v8 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_unlock(48, &p_parent->content_lock);
    return v4;
}

unsigned int MetadataContent::getLastSegment(const int i_current_stream) {
    //MetadataManager *v2; // eax@1
    metadata_stream_t *v3; // eax@4
    const metadata_stream_t *v4; // esi@4
    array_t<metadata_segment_t> *v5; // eax@5
    int v6; // eax@7
    metadata_segment_t *v7; // eax@7
    unsigned int v8; // esi@8
    //MetadataManager *v9; // eax@9

    //v2 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_lock(49, &p_parent->content_lock);
    if ( this->p_content
         && (v3 = getStream(this->p_content, i_current_stream), (v4 = v3) != 0)
         && (v5 = v3->p_segments) != 0
         && array_count(v5) > 0
         && (v6 = array_count(v4->p_segments), (v7 = getSegment(v4, v6 - 1)) != 0) )
    {
        v8 = v7->i_sequence;
    }
    else
    {
        v8 = 0x80000001;
    }
    //v9 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_unlock(49, &p_parent->content_lock);
    return v8;
}

int MetadataContent::getPieceSize(const int i_current_stream, const int i_segment,
                                  const int i_piece_id) {
    //MetadataManager *v4; // eax@1
    int v5; // esi@3
    const metadata_stream_t *v6; // eax@4
    const metadata_segment_t *v7; // eax@5
    metadata_piece_t *v8; // eax@6
    //int v9; // edi@7
    //MetadataManager *v10; // eax@8

    //v4 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_lock(50, &p_parent->content_lock);
    v5 = 0;
    if ( this->p_content )
    {
        v6 = getStream(this->p_content, i_current_stream);
        if ( v6 )
        {
            v7 = findSegment(v6, i_segment);
            if ( v7 )
            {
                v8 = findPiece(v7, i_piece_id);
                if ( v8 )
                {
                    v5 = v8->i_size;
                    //v9 = HIDWORD(v8->i_size);
                }
            }
        }
    }
    //v10 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_unlock(50, &p_parent->content_lock);
    return v5;
}

MetadataContent::MetadataContent(goalbit_t_0 *p_goalbit, MetadataManager *p_manager) {
    //MetadataParser *v3; // esi@1

    p_this = p_goalbit;
    p_parent = p_manager;
    //v3 = (MetadataParser *)operator new(4u);
    //v3 = new MetadataParser(this->p_this);
    p_parser = new MetadataParser(p_this);
    p_content = 0;
    p_pieces = hash_new();
    t_next_metadata_update = mdate();
}

array_t<metadata_piece_t> *MetadataContent::getSegmentPieces(const int i_current_stream, const int i_segment) {

    //MetadataManager *v3; // eax@1
    array_t<metadata_piece_t>* v4; // edi@3
    const metadata_stream_t *v5; // eax@4
    metadata_segment_t *v6; // eax@5
    //const metadata_segment_t *v7; // esi@5
    int v8; // ebx@7
    int i; // ST04_4@8
    //const metadata_piece_t *v10; // eax@8
    //metadata_piece_t *v11; // eax@8
    //MetadataManager *v12; // eax@10

    //v3 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_lock(51, &p_parent->content_lock);

    v4 = 0;
    if ( this->p_content )
    {
        v5 = getStream(this->p_content, i_current_stream);
        if ( v5 )
        {
            v6 = findSegment(v5, i_segment);
            //v7 = v6;
            if ( v6 )
            {
                if ( v6->p_pieces )
                {
                    v8 = 0;
                    v4 = array_new<metadata_piece_t>();
                    while ( v8 < array_count(v6->p_pieces) )
                    {
                        i = v8++;
                        //v10 = getPiece(v6, i);
                        //v11 = copyPiece(getPiece(v6, i));
                        array_append(v4, copyPiece(getPiece(v6, i)));
                    }
                }
            }
        }
    }

    //v12 = this->p_parent;
    if ( p_parent )
        Mypthread_mutex_unlock(51, &p_parent->content_lock);
    return v4;
}

MetadataContent::~MetadataContent() {
    if ( this->p_content ) {
        freeContent(this->p_content);
        this->p_content = 0;
    }
    if ( this->p_parser )
    {
        delete this->p_parser;
    }
    if ( this->p_pieces )
        hash_destroy(this->p_pieces, free_piece, this->p_this);
}

bool btBitField::IsSet(size_t i_row, size_t idx) {
    bool result; // al@2
    //size_t v4; // edi@3
    //unsigned int v5; // eax@3
    //bool v6; // cf@3
    //size_t v7; // edi@5
    //unsigned int v8; // eax@5
    unsigned char *v10; // ebx@9
    //goalbit_t_0 *v11; // eax@10
    //int v12; // esi@10
    unsigned int v13; // edx@11
    int v14; // edi@19
    int v15; // ebx@19
    unsigned int v16; // edx@20
    //goalbit_structure_t *v17; // ecx@23

    result = 0;
    if ( i_row == 0x80000001 || i_row < this->i_rows )
    {
        if (idx < this->i_min_id) {
            return 0;
        }
        if (idx > this->i_max_id) {
            return 0;
        }
        result = 0;
        if ( this->b )
        {
            if ( i_row == 0x80000001 )
            {
                if ( this->i_rows )
                {
                    v14 = 0;
                    v15 = 0;
                    do
                    {
                        //v17 = this->p_this->p_structure;
                        if ( this->b_hls_window )
                            v16 = idx % p_this->p_structure->i_hls_win_length;
                        else
                            v16 = idx % p_this->p_structure->i_p2p_win_length;
                        if (this->b[v14][v16 >> 3] == 0xFF) {
                            this->b[v14][v16 >> 3] = 0xFF;
                        }
                        if ( this->b[v14][v16 >> 3] & BIT_HEX[v16 & 7] )
                            return 1;
                        v14 = ++v15;
                    } while ( v15 != this->i_rows );
                    result = 0;
                }
            }
            else
            {
                v10 = this->b[i_row];
                if ( v10 )
                {
                    //v17 = this->p_this->p_structure;
                    //v12 = *(_DWORD *)idx;
                    if ( this->b_hls_window )
                        v13 = idx % p_this->p_structure->i_hls_win_length;
                    else
                        v13 = idx % p_this->p_structure->i_p2p_win_length;
                    result = (v10[v13 >> 3] & BIT_HEX[v13 & 7]) != 0;
                }
            }
        }
    }
    return result;
}

void btBitField::UpdateWindow(size_t i_new_offset) {

    RemoteUpdateWindow(i_new_offset);
    this->i_min_id = i_new_offset;
    if ( this->b_hls_window == false ) {
        this->i_max_id = (i_new_offset + p_this->p_structure->i_p2p_win_length - 1);
    } else {
        this->i_max_id = (i_new_offset + p_this->p_structure->i_hls_win_length - 1);
    }
    print_wrn(__func__, "i_min_id: %u -> i_max_id: %u", i_new_offset, this->i_max_id);
}

void btBitField::RemoteUpdateWindow(size_t i_new_offset) {
    size_t v4; // ebx@4
    size_t win_length; // eax@7
    size_t v9; // ebx@9
    int v17; // [sp+Ch] [bp-20h]@9
    unsigned int i; // [sp+10h] [bp-1Ch]@3

    if ( !this->b || !this->i_rows ) {
        return;
    }
    i = 0;
    do {
        v4 = this->i_min_id;
        while (v4 <= i_new_offset) {
            UnSet(i, v4++);
        }
        win_length = p_this->p_structure->i_p2p_win_length;
        if (this->b_hls_window)
            win_length = p_this->p_structure->i_hls_win_length;
        v9 = this->i_max_id;
        v17 = i_new_offset + win_length-1;
        while (v9 >= v17) {
            UnSet(i, v9--);
        }
    } while ( ++i < this->i_rows );
    return;
}

void btBitField::UnSet(size_t i_row, size_t idx) {
    int64_t v3; // rax@1
    //size_t v4; // esi@2
    //unsigned int v5; // edx@2
    //size_t v6; // esi@4
    //unsigned int v7; // edx@4
    unsigned __int8 **v8; // edx@6
    unsigned __int8 *v9; // ebx@7
    //goalbit_structure_t *v10; // edi@8
    unsigned int v11; // edx@9
    unsigned int v12; // eax@10
    unsigned __int8 v13; // dl@10
    unsigned __int8 *v14; // ebx@10

    v3 = idx;
    if ( this->i_rows > i_row )
    {
        if (idx < this->i_min_id) {
            return;
        }
        if (idx > this->i_max_id) {
            return;
        }
        v8 = this->b;
        if ( v8 )
        {
            v9 = v8[i_row];
            if ( v9 ) {
                if ( this->b_hls_window )
                    v11 = v3 % p_this->p_structure->i_hls_win_length;
                else
                    v11 = v3 % p_this->p_structure->i_p2p_win_length;
                v12 = v11;
                v13 = BIT_HEX[v11 & 7];
                v14 = &v9[v12 >> 3];
                if ( v13 & *v14 )
                {
                    *v14 &= ~v13;
                    --this->nset[i_row];
                }
            }
        }
    }
}

btBitField::btBitField(goalbit_t_0 *p_goalbit, size_t i_row_count, bool b_hls_win) {
    //goalbit_structure_t *v4; // edx@5
    unsigned int v6; // eax@6
    size_t v7; // eax@6
    bool v8; // zf@6
    size_t v10; // edi@9
    size_t v11; // edx@9
    size_t v12; // ST18_4@10
    unsigned __int8 **v13; // ebx@10
    unsigned __int8 *v14; // eax@10
    //goalbit_structure_t *v18; // edx@12

    this->p_this = p_goalbit;
    this->b_hls_window = b_hls_win;
    if ( p_goalbit && i_row_count )
    {
        if ( b_hls_win )
        {
            //v18 = p_goalbit->p_structure;
            this->i_min_id = p_goalbit->p_structure->i_hls_win_offset;
            this->i_max_id = (this->i_min_id + p_goalbit->p_structure->i_hls_win_length - 1);
            this->nbits = p_goalbit->p_structure->i_hls_win_length;
        }
        else
        {
            //v4 = p_goalbit->p_structure;
            this->i_min_id = p_goalbit->p_structure->i_p2p_win_offset;
            this->i_max_id = (this->i_min_id + p_goalbit->p_structure->i_p2p_win_length - 1);
            this->nbits = p_goalbit->p_structure->i_p2p_win_length;
        }
        v6 = this->nbits;
        this->i_rows = i_row_count;
        v7 = v6 >> 3;
        v8 = (this->nbits & 7) == 0;
        this->nbytes = v7;
        if ( !v8 )
            this->nbytes = v7 + 1;
        this->b = (unsigned __int8 **)malloc(sizeof(void*) * this->i_rows);
        if ( this->i_rows )
        {
            v10 = 0;
            v11 = 0;
            do
            {
                v12 = v11;
                v13 = &this->b[v11];
                v14 = (unsigned __int8 *)malloc(this->nbytes);
                *v13 = v14;
                ++v10;
                memset(this->b[v12], 0, this->nbytes);
                v11 = v10;
            } while ( this->i_rows > v10 );
        }
        this->nset = (size_t *)malloc(sizeof(void*) * this->i_rows);
        memset(this->nset, 0, sizeof(void*) * this->i_rows);
    }
    else
    {
        this->i_min_id = 0;
        this->i_max_id = 0;
        this->i_rows = 0;
        this->nbits = 0;
        this->nbytes = 0;
        this->nset = 0;
        this->b = 0;
    }
}

void btBitField::Set(size_t i_row, size_t idx) {
    unsigned int v11; // edx@11
    unsigned __int8 *v12; // ecx@12
    unsigned __int8 v13; // al@12

    if ( !this->b_hls_window ) {
        print_dbg(goalbitp2p, "[%s btBitField] Set (#%u,%u)", "P2P", idx, i_row);
    } else {
        print_dbg(__func__, "[%s btBitField] Set (#%u,%u)", "HLS", idx, i_row);
    }
    if ( i_row < this->i_rows ) {
        if (idx < this->i_min_id) {
            return;
        }
        if (idx > this->i_max_id) {
            return;
        }
        if ( this->b ) {
            if ( this->b[i_row] ) {
                if ( this->b_hls_window )
                    v11 = idx % p_this->p_structure->i_hls_win_length;
                else
                    v11 = idx % p_this->p_structure->i_p2p_win_length;
                v12 = &this->b[i_row][v11 >> 3];
                v13 = BIT_HEX[v11 & 7];
                if ( !(v13 & *v12) )
                {
                    *v12 |= v13;
                    if (*v12 == 0xFF && v13 != 1) {
                        *v12 = 0xFF;
                    }
                    ++this->nset[i_row];
                }
            }
        }
    }
}

btBitField::btBitField(btBitField *bf) {
    size_t v10; // edi@8

    this->i_min_id = bf->i_min_id;
    this->p_this = bf->p_this;
    this->b_hls_window = bf->b_hls_window;
    this->i_max_id = bf->i_max_id;
    this->i_rows = bf->i_rows;
    if ( bf->b_hls_window ) {
        this->nbits = p_this->p_structure->i_hls_win_length;
    } else {
        this->nbits = p_this->p_structure->i_p2p_win_offset;
    }
    this->nbytes = this->nbits >> 3;
    if ( this->nbits & 7 )
        this->nbytes = (this->nbits >> 3) + 1;
    this->b = (unsigned __int8 **)malloc(sizeof(void*) * bf->i_rows);
    this->nset = (size_t *)malloc(sizeof(void*) * this->i_rows);
    if ( !this->nset || !this->b )
    {
        //v17 = (_DWORD *)_cxa_allocate_exception(4);
        //*v17 = 9;
        //_cxa_throw(v17, &`typeinfo for'int, 0);
    }
    memcpy(this->nset, bf->nset, 4 * this->i_rows);
    if ( this->i_rows ) {
        v10 = 0;
        do
        {
            if ( bf->b[v10] )
            {
                this->b[v10] = (unsigned __int8 *)malloc(this->nbytes);
                memcpy(this->b[v10], bf->b[v10], this->nbytes);
            } else {
                this->b[v10] = 0;
            }
        } while ( ++v10 < this->i_rows );
    }
}

void btBitField::Xor(const btBitField *bf) {
    size_t v6; // eax@6
    size_t v8; // ebp@12
    size_t v11; // eax@14
    size_t v12; // [sp+0h] [bp-18h]@4

    if ( bf && this->b && bf->b )
    {
        v12 = 0;
        if ( this->i_rows )
        {
            do
            {
                if ( this->nbytes )
                {
                    v6 = 0;
                    do
                    {
                        this->b[v12][v6] &= ~bf->b[v12][v6];
                    }
                    while ( ++v6 < this->nbytes );
                }
            }
            while ( ++v12 < this->i_rows );
        }
        if ( this->b )
        {
            if ( this->nset )
            {
                if ( this->i_rows )
                {
                    v8 = 0;
                    do
                    {
                        this->nset[v8] = 0;
                        if ( this->nbits )
                        {
                            v11 = 0;
                            do
                            {
                                if ( this->b[v8][(signed int)v11 >> 3] & BIT_HEX[v11 & 7] )
                                    this->nset[v8]++;
                            } while ( ++v11 < this->nbits );
                        }
                    } while ( ++v8 < this->i_rows );
                }
            }
        }
    }
}

void btBitField::UnsetSmallerPieces(size_t idx) {
    size_t v2 = this->i_min_id;
    if ( v2 <= idx && this->b && this->nset && this->i_rows )
    {
        size_t v3 = 0;
        while ( 1 )
        {
            while ( v2 <= idx )
            {
                UnSet(v3, v2);
                v2++;
            }
            v3++;
            if ( v3 >= this->i_rows )
                break;
            v2 = this->i_min_id;
        }
    }
}

void btBitField::UnSet(size_t idx) {
    size_t v6; // eax@8
    size_t v7; // ebx@8

    if (idx < this->i_min_id) {
        return;
    }
    if (idx > this->i_max_id) {
        return;
    }
    if ( this->b && this->nset && this->i_rows )
    {
        v6 = 0;
        v7 = 0;
        do
        {
            ++v7;
            UnSet(v6, idx);
            v6 = v7;
        } while ( v7 < this->i_rows );
    }
}

void btBitField::UnsetBiggerPieces(size_t idx) {
    size_t v3; // ebp@6
    int j; // ST08_4@7
    unsigned int i; // [sp+Ch] [bp-14h]@6

    if ( idx != 0x80000001 )
    {
        size_t v2 = this->i_max_id;
        if ( idx <= v2 && this->b && this->nset && this->i_rows )
        {
            v3 = 0;
            i = 0;
            while ( 1 )
            {
                while ( v2 >= idx )
                {
                    j = v2;
                    v2--;//v2 = (v2 - 1);
                    UnSet(v3, j);
                }
                v3 = ++i;
                if ( i >= this->i_rows )
                    break;
                v2 = this->i_max_id;
            }
        }
    }
}

bool btBitField::IsEmpty(size_t i_row) {
    bool result; // al@1
    unsigned __int8 **v3; // esi@1
    size_t *v4; // edx@2
    size_t v5; // ecx@8
    int v6; // eax@10

    result = 1;
    v3 = this->b;
    if ( v3 )
    {
        v4 = this->nset;
        if ( v4 )
        {
            if ( i_row == 0x80000001 )
            {
                v5 = this->i_rows;
                if ( v5 )
                {
                    result = 0;
                    if ( !*v4 )
                    {
                        v6 = 0;
                        do
                        {
                            if ( ++v6 == v5 )
                                return 1;
                        }
                        while ( !v4[v6] );
                        result = 0;
                    }
                }
            }
            else if ( i_row < this->i_rows )
            {
                if ( v3[i_row] )
                    result = v4[i_row] == 0;
            }
        }
    }
    return result;
}

int btBitField::SelectRandomUniformID(size_t i_row) {
    for (int i = this->i_min_id; i < this->i_max_id; ++i) {
        if (IsSet(i_row, i)) {
            return i;
        }
    }
    return 0;
//    unsigned int v2; // ecx@1
//    unsigned int v4; // eax@5
//    unsigned int v5; // ecx@5
//    int v7; // esi@5
//    unsigned int v8; // edx@5
//    bool v9; // cf@5
//    bool v10; // zf@5
//    //unsigned int v11; // eax@7
//    unsigned int v13; // edx@14
//
//    if ( this->i_rows <= i_row )
//        return 0;
//    if ( !this->b || !this->b[i_row] || !this->nset )
//        return 0;
//    v4 = random();
//    v5 = this->nset[i_row];
//    v7 = v5 - 1;
//    v8 = v4 % v5 + 1;
//    v9 = v5 < v8;
//    v10 = v5 == v8;
//    v2 = this->i_min_id;
//    if ( !v9 && !v10 )
//        v7 = v8;
//        while (v2 < this->i_max_id) {
//            if ( v7 < 0 )
//                return v2 - 1;
//            if ( this->b_hls_window )
//                v13 = v2 % this->p_this->p_structure->i_hls_win_length;
//            else
//                v13 = v2 % this->p_this->p_structure->i_p2p_win_length;
//            v7 = ((BIT_HEX[v13 & 7] & this->b[i_row][v13 >> 3]) < 1u) + v7 - 1;
//            ++v2;
//        }
//    if ( v7 >= 0 )
//        return v2;
//    return v2 - 1;
}

void btBitField::print() {
    size_t v4; // ebx@2
    size_t v9; // edx@8
    int i_prev_aux; // ST18_4@10
    int v14; // eax@12
    int v15; // ST18_4@13
    unsigned int i; // [sp+1Ch] [bp-20h]@7

    print_wrn(__func__, "\n\n[BitField] print -------------------------------------------------------------\n");
    print_wrn(__func__, "   i_rows   = %d\n", this->i_rows);
    print_wrn(__func__, "   nbytes   = %d\n", this->nbytes);
    print_wrn(__func__, "   nbits    = %d\n", this->nbits);
    print_wrn(__func__, "   i_min_id = %d\n", this->i_min_id);
    print_wrn(__func__, "   i_max_id = %d\n", this->i_max_id);
    print_wrn(__func__, "   b_hls    = %d\n", this->b_hls_window);

    if ( this->nset )
    {
        v4 = 0;
        if ( !this->i_rows ) {
            print_wrn(__func__, "[BitField] -------------------------------------------------------------------\n");
            return;
        }
        do
        {
            print_wrn(__func__, "   nset[%d] = %d\n", v4, this->nset[v4]);
        } while (++v4 < this->i_rows);
    }
    if ( this->i_rows )
    {
        i = 0;
        do
        {
            i_prev_aux = -1;
            print_wrn(__func__, "Quality: %d ****************", i);
            v9 = this->i_min_id;
            if ( v9 < this->i_max_id )
            {
                do
                {
                    v14 = IsSet( i, v9);
                    if ( i_prev_aux == v14 ) {
                        //print_wrn(__func__, ".");
                        continue;
                    }
                    i_prev_aux = v14;
                    print_wrn(__func__, "\n%d:\t%d", v9, v14);
                } while (++v9 < this->i_max_id);
            }
        }
        while ( ++i < this->i_rows );
    }
    print_wrn(__func__, "[BitField] -------------------------------------------------------------------\n");
}

char *btBitField::GetBitfieldData(size_t i_offset, size_t i_length, size_t *i_size)
{

    if ( !this->b )
    {
        return 0;
    }
    if ( !this->i_rows ) {
        return 0;
    }
    size_t i_max_size = i_length * this->i_rows * 8;
    char* p_buffer_aux = (char *)malloc(i_max_size);
    memset(p_buffer_aux, 0, i_max_size);
    if ( this->i_rows )
    {
        int i_row = 0;
        char* psz = p_buffer_aux;
        int v29 = i_offset + i_length;
        do
        {
            if ( this->i_min_id == i_offset )
            {
                int v26 = i_length;
                if ( this->i_max_id < v29 )
                    v26 = this->nbits;
                _read_bitfield(i_row, psz, 0, v26);
                psz += v26 * 8;
                continue;
            }
            if ( this->i_min_id > i_offset ) {
                if ( this->i_min_id < v29 )
                {
                    int v13;
                    if ( this->i_max_id < v29 )
                    {
                        v13 = this->nbits;
                    }
                    else
                    {
                        v13 = i_length-(this->i_min_id-i_offset);
                    }
                    this->_read_bitfield(i_row, &psz[(this->i_min_id-i_offset)*8], 0, v13);
                    psz += v13 * 8;
                }
                continue;
            }
            if ( i_offset < this->i_max_id )
            {
                int v20 = i_length;
                if ( this->i_max_id < v29 )
                {
                    v20 -= v29 - this->i_max_id;
                }
                _read_bitfield(i_row, psz, i_offset - this->i_min_id, v20);
                psz += v20 * 8;
            }
        } while (++i_row < this->i_rows);
    }
    if ( !i_size )
        i_size = (size_t *)malloc(sizeof(size_t));
    *i_size = i_max_size;
    return p_buffer_aux;
}

void btBitField::_read_bitfield(size_t i_row, char *psz, size_t i_off, size_t i_length) {
    size_t v7; // edi@2
    size_t i_offset_bytes; // [sp+14h] [bp-28h]@7

    unsigned int v5 = i_length;
    if ( this->b_hls_window )
    {
        v7 = this->nbits - (this->i_min_id + i_off) % this->p_this->p_structure->i_hls_win_length;
    }
    else
    {
        v7 = this->nbits - (this->i_min_id + i_off) % this->p_this->p_structure->i_p2p_win_length;
    }
    if ( v7 > i_length )
        v7 = i_length;
    if ( v7 )
    {
        if ( this->b_hls_window )
            i_offset_bytes = (this->i_min_id + i_off) % p_this->p_structure->i_hls_win_length >> 3;
        else
            i_offset_bytes = (this->i_min_id + i_off) % p_this->p_structure->i_p2p_win_length >> 3;
        v5 = i_length - v7;
        memcpy(psz, &this->b[i_row][i_offset_bytes], v7 >> 3);
    }
    if ( v5 )
        memcpy(&psz[v7 >> 3], this->b[i_row], v5 >> 3);
}

void btBitField::SetBitfieldData(char *p_buffer, size_t i_offset, size_t i_length)
{
    unsigned int v8; // ebp@6
    size_t v9; // edi@8
    unsigned int v10; // eax@10
    unsigned int v12; // ecx@14
    unsigned int v13; // eax@22
    unsigned int v14; // edx@22
    bool v15; // cl@22
    unsigned int v16; // eax@25
    size_t v18; // ebp@32
    size_t v21; // eax@34
    size_t v25; // edi@41
    bool v29; // [sp+2Ch] [bp-30h]@14
    size_t i_bitfield_offset; // [sp+38h] [bp-24h]@6

    if (p_buffer && this->i_rows && i_length) {
        if (!this->b) {
            this->b = (unsigned char**)malloc(sizeof(int*) *this->i_rows);
            if (this->i_rows) {
                v25 = 0;
                do {
                    this->b[v25++] = (unsigned __int8 *) malloc(this->nbytes);
                } while (v25 >= this->i_rows);
            }
        }
        if (this->b) {
            i_bitfield_offset = 0;
            v8 = (i_length / this->i_rows)/8;
            if (v8 > this->nbits)
                v8 = this->nbits;
            v9 = 0;
            do {
                v10 = i_offset - this->i_min_id;
                v13 = v8 + i_offset;
                if (i_offset <= this->i_min_id) {
                    i_bitfield_offset = (this->i_min_id-i_offset)*8;
                } else {
                    p_buffer += v10*8;
                    v8 -= v10*8;
                }
                if (this->i_max_id < v13) {
                    v8 -= (v13-this->i_max_id)*8;
                }
                if (!v8) {
                    continue;
                }
                _write_bitfield(v9, p_buffer, i_bitfield_offset, v8);
                p_buffer += v8 * 8;
            } while (v9++ >= this->i_rows);
        }
        if (this->b) {
            if (this->nset) {
                if (this->i_rows) {
                    v18 = 0;
                    do {
                        this->nset[v18] = 0;
                        if (this->nbits) {
                            v21 = 0;
                            do {
                                if (this->b[v18][v21] & BIT_HEX[v21 & 7])
                                    this->nset[v18]++;
                            } while (++v21 < this->nbits);
                        }
                    } while (++v18 < this->i_rows);
                }
            }
        }
    }
}

void btBitField::_write_bitfield(size_t i_row, char *psz, size_t i_off, size_t i_length) {
    unsigned int v5; // esi@1
    size_t v8; // edi@2
    size_t v9; // ebp@7

    v5 = i_length;
    if (this->b_hls_window) {
        v8 = this->nbits - (i_off + this->i_min_id) % this->p_this->p_structure->i_hls_win_length;
    } else {
        v8 = this->nbits - (this->i_min_id + i_off) % this->p_this->p_structure->i_p2p_win_length;
    }
    if (v8 > i_length)
        v8 = i_length;
    if (v8) {
        if (this->b_hls_window)
            v9 = (this->i_min_id + i_off) % this->p_this->p_structure->i_hls_win_length >> 3;
        else
            v9 = (this->i_min_id + i_off) % this->p_this->p_structure->i_p2p_win_length >> 3;
        v5 = i_length - v8;
        memcpy(&this->b[i_row][v9], psz, v8 >> 3);
    }
    if (v5)
        memcpy(this->b[i_row], &psz[v8 >> 3], v5 >> 3);
}

unsigned int parse_timestamp(unsigned __int8 *a1)
{
return (unsigned int)((((a1[3] << 8) | a1[4]) >> 1) | ((*a1 & 0xF) >> 1 << 30) | (((a1[1] << 8) | a1[2]) >> 1 << 15));
}

double MpegTS::parseContent(block_t *p_block) {
    int v2; // ebp@1
    //unsigned short* v3; // edi@3
    //size_t v4; // eax@3
    //unsigned int v5; // edx@3
    //char *v6; // eax@3
    //char *v7; // esi@3
    //int v8; // eax@7
    //unsigned short *v9; // edi@7
    //char *v10; // esi@7
    int v11; // ebx@11
    //unsigned short v12; // ax@11
    short v13; // ax@11
    int v14; // edx@11
    double_t v15; // ebx@23
    long double v16; // fst6@23
    float v17; // ST00_4@24
    double_t result; // eax@25
    //int v19; // edx@25
    //unsigned char *v20; // eax@27
    //unsigned char *v21; // edi@29
    //unsigned short *v22; // esi@29
    //unsigned int v23; // edx@29
    signed int v24; // eax@33
    //unsigned short *v25; // edi@33
    //unsigned short *v26; // esi@33
    //unsigned short v27; // ax@6
    //unsigned short *v28; // eax@36
    //unsigned short *v29; // edi@38
    //unsigned short *v30; // esi@38
    //unsigned int v31; // edx@38
    //signed int v32; // eax@42
    //unsigned short *v33; // edi@42
    //unsigned short *v34; // esi@42
    //signed long v35; // rax@50
    //unsigned int v36; // ebx@50
    //unsigned int v37; // esi@50
    //unsigned int v38; // edx@50
    //char *v39; // edi@50
    //char *v40; // edi@54
    long double v41; // fst7@58
    long double v42; // fst6@64
    //unsigned short v43; // ax@41
    //unsigned short v44; // ax@32
    //signed long v45; // rax@75
    //unsigned int v46; // ebx@75
    //unsigned int v47; // esi@75
    //unsigned int v48; // edx@75
    //char *v49; // edi@75
    //char *v50; // edi@79
    long double v51; // fst6@89
    long long v52; // [sp+18h] [bp-194h]@0
    //char *v53; // [sp+20h] [bp-18Ch]@0
    uint8_t *p_piece_buffer; // [sp+2Ch] [bp-180h]@1
    float f_max_time; // [sp+34h] [bp-178h]@2
    float f_min_time; // [sp+38h] [bp-174h]@2
    float f_last_pts; // [sp+3Ch] [bp-170h]@2
    bool b_found_pat; // [sp+42h] [bp-16Ah]@2
    bool b_found_pmt; // [sp+43h] [bp-169h]@2
    int i_current_state; // [sp+44h] [bp-168h]@2
    //unsigned int v61; // [sp+48h] [bp-164h]@2
    float f_current_pts; // [sp+5Ch] [bp-150h]@58
    float f_current_ptsa; // [sp+5Ch] [bp-150h]@83
    unsigned char v64[200]; // [sp+67h] [bp-145h]@3
    //unsigned short __x; // [sp+68h] [bp-144h]@4
    //unsigned char v66; // [sp+6Ah] [bp-142h]@11
    //unsigned char v67; // [sp+6Bh] [bp-141h]@14
    //char v68[179]; // [sp+70h] [bp-13Ch]@50
    char tempbuff[100]; // [sp+123h] [bp-89h]@2
    long long timestamp;//unsigned char timestamp[5]; // [sp+187h] [bp-25h]@50
    //int v71; // [sp+18Ch] [bp-20h]@1

    //v71 = *MK_FP(__GS__, 0x14);
    p_piece_buffer = block_Content(p_block);
    v2 = block_Total_Size(p_block);
    if ( v2 <= 0xBB )
    {
        //v15 = 0;
        //f_max_time = 0.0;
        //f_min_time = 0.0;
        return 0;
    }
    f_last_pts = 0.0;
    f_max_time = 0.0;
    //v53 = tempbuff;
    i_current_state = 1;
    f_min_time = 0.0;
    b_found_pmt = 0;
    b_found_pat = 0;
    //v61 = (unsigned int)tempbuff[0] & 1;
    do
    {
        memcpy(v64, &p_piece_buffer[block_Total_Size(p_block) - v2], 0xBC);
        v2 -= 0xBC;
        v11 = 0xBC;
        //v12 = *(unsigned short*)(v64+1);
        //msbtolsb(&v12, v64+1, 2);
        //v13 = v12 & 0x1FFF;
        v13 = htons(*(unsigned short*)(v64+1)) & 0x1FFF;
        v14 = (*(unsigned char*)(v64+3) >> 4) & 3;
        if ( v14 )
        {
            (v11) = 4;
            if ( (_BYTE)v14 != 1 )
            {
                (v11) = 0xBCu;
                if ( (_BYTE)v14 != 2 )
                    v11 = *(unsigned char*)(v64+4) + 5;
            }
        }
        if ( v13 || b_found_pat )
        {
            if ( v13 == this->i_pmt_pid && !b_found_pmt )
            {
                print_dbg(__func__, "[MpegTS] parsePSI - Found new PMT");
                //v20 = this->p_pmt;
                if ( !this->p_pmt )
                {
                    this->p_pmt = (unsigned __int8 *)malloc(0xBC);
                    //this->p_pmt = v20;
                }
                v24 = 0;
                memcpy(this->p_pmt, v64, 0xBC);
                b_found_pmt = 1;
            }
        }
        else
        {
            print_dbg(__func__, "[MpegTS] parsePSI - Found new PAT");
            parsePAT(v64+1+v11);
            if ( !this->p_pat )
            {
                this->p_pat = (u_char*)malloc(0xBC);
            }
            memcpy(this->p_pat, v64, 0xBC);
            b_found_pat = 1;
        }
        LABEL_18:
        if ( v11 <= 0xA7 && !v64[v11] && !v64[v11+1] && v64[v11+2] == 1 )
        {
            if ( v64[v11+3] >> 4 == 0xE )
            {
                //                *(_DWORD *)timestamp = *(_DWORD *)&v68[v11];
                //                timestamp[4] = v68[v11 + 4];
                //                v45 = ( __int64)((timestamp[2] | (timestamp[1] << 8)) >> 1) << 0xF;
                //                v46 = (v45 >> 16) | ((timestamp[4] | (timestamp[3] << 8)) >> 0x20) | (( __int64)((timestamp[0] & 0xF) >> 1) >> 2);
                //                v47 = v45 | ((timestamp[4] | (timestamp[3] << 8)) >> 1) | ((timestamp[0] & 0xF) >> 1 << 0x1E);
                timestamp = parse_timestamp(&v64[v11+9]);//msbtolsb(&timestamp, &v64[v11+9], 5);
                //v48 = 0x64;
                //v49 = tempbuff;
                //                if ( v61 )
                //                {
                //                    tempbuff[0] = 0;
                //                    v49 = &tempbuff[1];
                //                    (v48) = 0x63;
                //                }
                //                if ( (unsigned __int8)v49 & 2 )
                //                {
                //                    *(_WORD *)v49 = 0;
                //                    v48 -= 2;
                //                    v49 += 2;
                //                }
                memset(tempbuff, 0, sizeof(tempbuff));
                //                v50 = &v49[4 * (v48 >> 2)];
                //                if ( v48 & 2 )
                //                {
                //                    *(_WORD *)v50 = 0;
                //                    v50 += 2;
                //                }
                //                if ( v48 & 1 )
                //                    *v50 = 0;
                v52 = timestamp % 90000 / 9;
                sprintf(tempbuff, "%llu.%04llu", timestamp / 90000, v52);
                f_current_ptsa = strtod(tempbuff, 0);
                v41 = f_current_ptsa;
                if ( f_last_pts == 0.0 )
                    goto LABEL_98;
                if ( (long double)(signed int)abs((signed int)(v41 - f_last_pts)) > 4.0 )
                    goto LABEL_95;
                if ( i_current_state )
                    goto LABEL_98;
                if ( (long double)(signed int)abs((signed int)(v41 - f_max_time)) <= 20.0 )
                {
                    i_current_state = 1;
                    LABEL_98:
                    if ( f_min_time == 0.0 )
                    {
                        f_min_time = f_current_ptsa;
                    }
                    else
                    {
                        v51 = f_min_time;
                        if ( f_min_time > v41 )
                            v51 = f_current_ptsa;
                        f_min_time = v51;
                    }
                    if ( f_max_time != 0.0 && v41 <= f_max_time )
                        goto LABEL_70;
                    goto LABEL_94;
                }
            }
            else
            {
                if ( v64[v11+3] >> 5 != 6 )
                    continue;
                //                *(_DWORD *)timestamp = *(_DWORD *)&v68[v11];
                //                timestamp[4] = v68[v11 + 4];
                //                v35 = (__int64)((timestamp[2] | (timestamp[1] << 8)) >> 1) << 0xF;
                //                v36 = (v35 >> 16) | ((timestamp[4] | (timestamp[3] << 8)) >> 0x20) | (( __int64)((timestamp[0] & 0xF) >> 1) >> 2);
                //                v37 = v35 | ((timestamp[4] | (timestamp[3] << 8)) >> 1) | ((timestamp[0] & 0xF) >> 1 << 0x1E);
                timestamp = parse_timestamp(&v64[v11+9]);//msbtolsb(&timestamp, &v64[v11+9], 5);
                //v38 = 0x64;
                //v39 = tempbuff;
                //                if ( v61 )
                //                {
                //                    tempbuff[0] = 0;
                //                    v39 = &tempbuff[1];
                //                    (v38) = 0x63;
                //                }
                //                if ( (unsigned __int8)v39 & 2 )
                //                {
                //                    *(_WORD *)v39 = 0;
                //                    v38 -= 2;
                //                    v39 += 2;
                //                }
                memset(tempbuff, 0, sizeof(tempbuff));
                //                v40 = &v39[4 * (v38 >> 2)];
                //                if ( v38 & 2 )
                //                {
                //                    *(_WORD *)v40 = 0;
                //                    v40 += 2;
                //                }
                //                if ( v38 & 1 )
                //                    *v40 = 0;
                v52 = timestamp % 90000 / 9;
                sprintf(tempbuff, "%llu.%04llu", timestamp / 90000, v52);
                f_current_pts = strtod(tempbuff, 0);
                v41 = f_current_pts;
                if ( f_last_pts == 0.0 )
                    goto LABEL_102;
                if ( (long double)(signed int)abs((signed int)(v41 - f_last_pts)) > 4.0 )
                {
                    LABEL_95:
                    f_last_pts = v41;
                    i_current_state = 0;
                    continue;
                }
                if ( i_current_state )
                    goto LABEL_102;
                if ( (long double)(signed int)abs((signed int)(v41 - f_max_time)) <= 20.0 )
                {
                    i_current_state = 1;
                    LABEL_102:
                    if ( f_min_time == 0.0 )
                    {
                        f_min_time = f_current_pts;
                    }
                    else
                    {
                        v42 = f_min_time;
                        if ( f_min_time > v41 )
                            v42 = f_current_pts;
                        f_min_time = v42;
                    }
                    if ( f_max_time != 0.0 )
                    {
                        if ( v41 > f_max_time )
                            f_max_time = f_current_pts;
                        LABEL_70:
                        f_last_pts = v41;
                        continue;
                    }
                    LABEL_94:
                    f_max_time = v41;
                    f_last_pts = v41;
                    continue;
                }
            }
            f_last_pts = v41;
            f_max_time = v41;
            f_min_time = v41;
            i_current_state = 1;
        }
    } while ( v2 > 0xBB );
    v15 = 0;
    v16 = f_max_time - f_min_time;
    if ( v16 > 0.0 )
    {
        v17 = v16 * 1000000.0;
        v15 = roundf(v17);
    }
    //    msg_Dbg(
    //            this->p_this,
    //            "[MpegTS] parseContent - startPTS: %f, endPTS: %f, contentDuration: %u usecs.",
    //            f_min_time,
    //            f_max_time,
    //            v15,
    //            HIDWORD(v52),
    //            v53);
    result = v15;
    //v19 = *MK_FP(__GS__, 0x14) ^ v71;
    return result;
}

unsigned int MpegTS::getDuration(block_t *p_block, int i_offset, int i_length) {
    int v3; // ebp@1
    u_char *v4; // ebx@1
    unsigned int result; // eax@1
    long double v6; // fst7@2
    long double v7; // fst6@2
    long double v8; // fst5@2
    u_char *v9; // esi@3
    u_char *v10; // edi@3
    unsigned int v11; // eax@3
    signed int v12; // edx@7
    //u_char *v13; // edi@7
    //char *v14; // esi@7
    int v15; // eax@11
    int v16; // eax@14
    long double v17; // fst6@19
    float v18; // ST00_4@20
    //int v19; // edx@21
    //int16_t v20; // dx@6
    //u_char *v21; // eax@24
    //u_char v22; // dl@24
    //int v23; // esi@24
    //int v24; // ST68_4@24
    char *v25; // edi@24
    //int64_t v26; // rax@24
    //unsigned int v27; // esi@24
    unsigned int v28; // edx@24
    char *v29; // edi@28
    long double v30; // tt@31
    long double v31; // fst5@31
    long double v32; // fst7@31
    long double v33; // t0@32
    long double v34; // fst6@34
    float v35; // [sp+20h] [bp-1ACh]@33
    float v36; // [sp+30h] [bp-19Ch]@33
    float v37; // [sp+40h] [bp-18Ch]@33
    int i_current_state; // [sp+60h] [bp-16Ch]@2
    //int v39; // [sp+68h] [bp-164h]@24
    float f_current_pts; // [sp+7Ch] [bp-150h]@33
    u_char packet[188]; // [sp+87h] [bp-145h]@3
    char tempbuff[100]; // [sp+143h] [bp-89h]@24
    long long timestamp; // [sp+1A7h] [bp-25h]@24
    //int v44; // [sp+1ACh] [bp-20h]@1

    //v44 = *MK_FP(__GS__, 0x14);
    v3 = i_length;
    v4 = &block_Content(p_block)[i_offset];
    result = 0;
    if ( i_length <= 0xBB ) {
        //goto LABEL_21;
        return 0;
    }
    v6 = 0.0;
    v7 = 0.0;
    i_current_state = 1;
    v8 = 0.0;
    do
    {
        v9 = v4;
        v10 = packet;
        v11 = 0xBC;
        v12 = 0;
        memcpy(v10, v9, v11);
        v3 -= 0xBC;
        v15 = ((signed int)packet[3] >> 4) & 3;
        if ( v15 )
        {
            if ( (_BYTE)v15 == 1 )
            {
                v16 = 4;
            }
            else
            {
                if ( (_BYTE)v15 == 2 )
                    goto LABEL_18;
                v16 = packet[4] + 5;
                if ( v16 > 0xA7 )
                    goto LABEL_18;
            }
            if ( !packet[v16] && !packet[v16 + 1] && packet[v16 + 2] == 1 && (signed int)packet[v16 + 3] >> 4 == 0xE )
            {
                //                v21 = &packet[v16 + 9];
                //                *(_DWORD *)timestamp = *(_DWORD *)v21;
                //                v22 = v21[4];
                //                timestamp[4] = v22;
                //                v23 = ((v22 | (timestamp[3] << 8)) >> 1) | ((timestamp[0] & 0xF) >> 1 << 0x1E);
                //                v24 = ((v22 | (timestamp[3] << 8)) >> 0x20) | (( __int64)((timestamp[0] & 0xF) >> 1) >> 2);
                //                v25 = tempbuff;
                //                v26 = ( __int64)((timestamp[2] | (timestamp[1] << 8)) >> 1) << 0xF;
                //                v27 = v26 | v23;
                //                v39 = (v26 >> 16) | v24;
                timestamp = parse_timestamp(&packet[v16 + 9]);//msbtolsb(&timestamp, &packet[v16 + 9], 5);
                v28 = 0x64;
                //                if ( (unsigned int)tempbuff & 1 )
                //                {
                //                    tempbuff[0] = 0;
                //                    v25 = &tempbuff[1];
                //                    (v28) = 0x63;
                //                }
                //                if ( (unsigned __int8)v25 & 2 )
                //                {
                //                    *(_WORD *)v25 = 0;
                //                    v28 -= 2;
                //                    v25 += 2;
                //                }
                memset(v25, 0, 4 * (v28 >> 2));
                v29 = &v25[4 * (v28 >> 2)];
                if ( v28 & 2 )
                {
                    *(_WORD *)v29 = 0;
                    v29 += 2;
                }
                if ( v28 & 1 )
                {
                    v30 = v8;
                    v31 = v6;
                    v32 = v30;
                    *v29 = 0;
                }
                else
                {
                    v33 = v8;
                    v31 = v6;
                    v32 = v33;
                }
                v35 = v31;
                v36 = v7;
                v37 = v32;
                sprintf(tempbuff, "%llu.%04llu", timestamp / 90000, 0ull);
                f_current_pts = strtod(tempbuff, 0);
                v6 = f_current_pts;
                if ( v35 == 0.0 )
                {
                    v34 = v37;
                }
                else
                {
                    v34 = v37;
                    if ( (long double)(signed int)abs((signed int)(v6 - v35)) > 4.0 )
                    {
                        v8 = v37;
                        v7 = v36;
                        i_current_state = 0;
                        goto LABEL_18;
                    }
                    if ( i_current_state )
                    {
                        i_current_state = 1;
                    }
                    else
                    {
                        i_current_state = 1;
                        if ( (long double)(signed int)abs((signed int)(v6 - v36)) > 20.0 )
                            goto LABEL_37;
                    }
                }
                if ( v34 == 0.0 )
                {
                    LABEL_37:
                    v7 = f_current_pts;
                    v8 = f_current_pts;
                    goto LABEL_18;
                }
                v8 = v34;
                v7 = f_current_pts;
            }
        }
        LABEL_18:
        v4 += 0xBC;
    }
    while ( v3 > 0xBB );
    result = 0;
    v17 = v7 - v8;
    if ( v17 > 0.0 )
    {
        v18 = v17 * 1000000.0;
        result = roundf(v18);
    }
    LABEL_21:
    //v19 = *MK_FP(__GS__, 0x14) ^ v44;
    return result;
}

MpegTS::MpegTS(goalbit_t_0 *p_goalbit) {
    this->p_pat = 0;
    this->p_this = p_goalbit;
    this->p_pmt = 0;
    this->i_pmt_pid = 0;
}

unsigned int get_bits48(u_char *buf, int byte_offset, int startbit, int bitlen)
{
    unsigned int v4; // esi@1
    //int v5; // ecx@2
    //u_char *v6; // ecx@4
    //__int64 v7; // ST00_8@4
    //unsigned int v8; // ecx@4
    //__int64 v9; // rax@4

    v4 = 0xFEFEFEFE;
    //    if ( bitlen <= 0x30 )
    //    {
    //        v5 = startbit + 7;
    //        if ( startbit >= 0 )
    //            v5 = startbit;
    //        v6 = &buf[byte_offset] + (v5 >> 3);
    //        v7 = (v6[5] << 8)
    //             + (v6[4] << 0x10)
    //             + ((__int64)v6[3] << 0x18)
    //        + __PAIR__(
    //                (__PAIR__(v6[2], v6[6]) + __PAIR__((unsigned int)v6[1] << 8, 0) + __PAIR__((unsigned int)*v6 << 0x10, 0)) >> 0x20,
    //                v6[6]);
    //        v8 = ((unsigned int)(startbit >> 0x1F) >> 0x1D)
    //             - (((_BYTE)startbit + ((unsigned int)(startbit >> 0x1F) >> 0x1D)) & 7)
    //             + 0x38
    //             - bitlen;
    //        v9 = v7 >> (v8 & 0x1F);
    //        if ( v8 & 0x20 )
    //            LODWORD(v9) = HIDWORD(v9);
    //        v4 = ((((bitlen & 0x20) == 0) << bitlen) - 1) & v9;
    //    }
    return v4;
}

//int get_bits(u_char *buf, int byte_offset, int startbit, int bitlen)
int get_bits(u_char *buf, int byte_offset, int startbit, int bitlen)
{
    unsigned __int8 *v4; // r1@1
    int v5; // r2@1
    signed int result; // r0@2
    unsigned int v7; // r1@3
    char v8; // r4@3

    v4 = &buf[byte_offset] + (startbit >> 3);
    v5 = startbit % 8;
    switch ( ((bitlen - 1) >> 3) + 1 )
    {
        case 0:
            return 0;
        case 1:
            v7 = (*v4 << 8) + v4[1];
            v8 = 16;
            goto LABEL_7;
        case 2:
            v7 = (*v4 << 16) + (v4[1] << 8) + v4[2];
            v8 = 24;
            goto LABEL_7;
        case 3:
            v7 = (*v4 << 24) + (v4[1] << 16) + (v4[2] << 8) + v4[3];
            v8 = 32;
        LABEL_7:
            result = ((1LL << bitlen) - 1) & (v7 >> (v8 - v5 - bitlen));
            break;
        case 4:
            result = get_bits48(v4, 0, v5, bitlen);
            break;
        default:
            result = -2;
            break;
    }
    return result;
}

void MpegTS::parsePAT(unsigned char *p_pat_content) {
    unsigned short v2; // ax@2

    if ( (get_bits(p_pat_content, 0, 0xC, 0xC) - 9) > 3 )
    {
        v2 = get_bits(p_pat_content + 8, 0, 0x13, 0xD);
        this->i_pmt_pid = v2;
        print_dbg(__func__, "[MpegTS] parsePAT - Found PMT PID: %u", v2);
    }
}

/*
 void hash_init(hash_t_0 *p_dict, int i_size)
 {
 int v2; // ebx@1
 hash_entry_t_0 **v3; // eax@2

 v2 = i_size;
 p_dict->p_entries = 0;
 if ( i_size > 0 )
 {
 v3 = (hash_entry_t_0 **)calloc(i_size, 4);
 p_dict->p_entries = v3;
 if ( !v3 )
 v2 = 0;
 }
 p_dict->i_size = v2;
 }

 hash_t_0 *hash_new()
 {
 hash_t_0 *v0; // eax@1
 hash_t_0 *v1; // ebx@1

 v0 = new hash_t_0();
 v1 = v0;
 if ( v0 )
 hash_init(v0, 5);
 return v1;
 }
 */

void* run_manifest_uploader(void *pvoid)
{

    if ( pvoid )
    {
        char value; // [sp+1Ch] [bp-10h]@4
        HLSManager *hlsManager = (HLSManager *)pvoid;
        // read hls content
        hlsManager->p_hls_content->ManifestUploaderRun();
        hlsManager->b_ready = false;
        pthread_exit(&value);
    }
    return 0;
}

void* run_segment_downloader(void* pvoid)
{
    HLSManager *p_data = (HLSManager*)pvoid;
    void *result; // eax@1
    char value; // [sp+1Ch] [bp-10h]@3

    result = p_data;
    if ( p_data )
    {
        // process p_events_queue
        p_data->p_hls_content->ContentDownloaderRun();
        pthread_exit(&value);
    }
    return result;
}

HLSManager::HLSManager(goalbit_t* p_goalbit)
{
    this->b_ready = false;
    this->p_this = p_goalbit;
    this->p_segment_requests = hash_new();
    this->p_segments_ready = array_new<quality_segmentid>();
    this->p_piece_requests = hash_new();
    this->p_pieces_ready = array_new<piece_ready_t>();
    pthread_mutex_init(&this->segments_lock, 0);
    this->p_hls_content = new HLSContent(this->p_this, this);
    pthread_mutex_init(&this->content_lock, 0);
    this->p_hls_manifest_thread = new pthread_t();// *)malloc(4);
    this->p_hls_segment_thread = new pthread_t();

    if ( pthread_create(this->p_hls_manifest_thread, 0, run_manifest_uploader, this) ) {
        print_err(__func__, "[HLSManager] Can't create HLS Manifest Uploader thread!!");
        this->p_hls_manifest_thread = 0;
    }
    if ( pthread_create(this->p_hls_segment_thread, 0, run_segment_downloader, this) ) {
        print_err(__func__, "[HLSManager] Can't create HLS Segments Downloader thread!!");
        this->p_hls_segment_thread = 0;
    }
    return;
}

bool HLSManager::isReady() {
    return this->b_ready;
}

char * get_segment_key(const int i_stream, const int i_segment)
{
    char *psz_key; // [sp+2Ch] [bp-10h]@1

    psz_key = 0;
    asprintf(&psz_key, "%d_%d", i_stream, i_segment);
    return psz_key;
}

int HLSManager::isSegmentRequestedOrReady(const int i_segment) {
    int v2; // ebx@2
    char *v3; // edi@3
    char v4; // ST18_1@3
    int i; // ebx@6
    quality_segmentid *v6; // eax@7
    signed int v7; // ebx@11

    if ( this->p_segments_ready || (v7 = 0, this->p_segment_requests) )
    {
        v2 = 0;
        Mypthread_mutex_lock(52, &this->segments_lock);
        while ( v2 < this->p_hls_content->getStreamCount() )
        {
            v3 = get_segment_key(v2, i_segment);
            v4 = hash_contains_key(this->p_segment_requests, v3);
            free(v3);
            if ( v4 )
            {
                //goto isSegmentRequestedOrReady_return;
                Mypthread_mutex_unlock(52, &this->segments_lock);
                return true;
            }
            ++v2;
        }
        for ( i = 0; i < array_count(this->p_segments_ready); ++i )
        {
            v6 = array_item_at_index(this->p_segments_ready, i);
            if ( v6->i_quality == i_segment ) {
                //goto LABEL_12;
                Mypthread_mutex_unlock(52, &this->segments_lock);
                return true;
            }
        }
        v7 = 0;
        Mypthread_mutex_unlock(52, &this->segments_lock);
    }
    return v7;
}

bool HLSManager::isLiveContent() {
    if ( this->b_ready && p_hls_content != 0 )
        return p_hls_content->isLiveContent();
    return false;
}

goalbit_qualities_desc_t_0 *HLSManager::exportStreamsDescription() {
    if ( b_ready && (p_hls_content) != 0 )
        return p_hls_content->exportStreamsDescription();
    return 0;
}

int HLSManager::getInitialSequence() {
    unsigned int result; // eax@1
    HLSContent *v2; // edx@2
    int v3; // ebx@3
    unsigned int v4; // ebp@3
    unsigned int v5; // edi@3
    int v6; // eax@4
    unsigned int v7; // eax@7
    int v8; // ebx@12

    result = -1;
    if ( this->b_ready )
    {
        v2 = this->p_hls_content;
        if ( v2 )
        {
            v3 = 0;
            v4 = 0;
            v5 = 0x80000001;
            /**
             * v4 <- Max
             * v5 <- Min
             */
            while ( v3 < v2->getStreamCount() )
            {
                v6 = this->p_hls_content->getMinSegment(v3);
                if ( v6 && v5 > v6 )
                    v5 = v6;
                v7 = this->p_hls_content->getMaxSegment(v3);
                if ( v7 && v4 < v7 )
                    v4 = v7;
                v2 = this->p_hls_content;
                ++v3;
            }
            v8 = v4 - 5;
            if ( v4 - v5 < 5 )
                v8 = v5;
            print_dbg(__func__, "[HLSManager] MIN=%d, MAX=%d, START=%d", v5, v4, v8);
            result = v8;
        }
    }
    return result;
}

bool HLSManager::hasSegmentsReady() {
    int v1; // esi@1

    Mypthread_mutex_lock(53, &this->segments_lock);
    v1 = array_count(this->p_segments_ready);
    Mypthread_mutex_unlock(53, &this->segments_lock);
    return v1 > 0;
}

template <typename T>
void array_remove(array_t<T> *p_array, int i_index)
{
    int v2; // eax@2
    int v3; // eax@4

    if ( i_index >= 0 )
    {
        v2 = p_array->i_count;
        if ( p_array->i_count > 1 )
        {
            memmove(&p_array->pp_elems[i_index], &p_array->pp_elems[i_index + 1], 4 * (v2 - i_index) - 4);
            v2 = p_array->i_count;
        }
        v3 = v2 - 1;
        p_array->i_count = v3;
        if ( !v3 )
        {
            free(p_array->pp_elems);
            p_array->pp_elems = 0;
        }
    }
}

quality_segmentid *HLSManager::getNextSegmentReady() {
    quality_segmentid *v1 = 0; // edi@1

    if ( this->p_segments_ready )
    {
        Mypthread_mutex_lock(54, &this->segments_lock);
        if ( array_count(this->p_segments_ready) > 0 )
        {
            v1 = array_item_at_index(this->p_segments_ready, 0);
            array_remove(this->p_segments_ready, 0);
        }
        Mypthread_mutex_unlock(54, &this->segments_lock);
    }
    return v1;
}

bool HLSManager::hasPiecesReady() {
    int v1; // esi@1

    Mypthread_mutex_lock(55, &this->segments_lock);
    v1 = array_count(this->p_pieces_ready);
    Mypthread_mutex_unlock(55, &this->segments_lock);
    return v1 > 0;
}

piece_ready_t *HLSManager::getNextPieceReady() {
    piece_ready_t *v1; // edi@1

    v1 = 0;
    if ( this->p_pieces_ready )
    {
        Mypthread_mutex_lock(56, &this->segments_lock);
        if ( array_count(this->p_pieces_ready) > 0 )
        {
            v1 = array_item_at_index(this->p_pieces_ready, 0);
            array_remove(this->p_pieces_ready, 0);
        }
        Mypthread_mutex_unlock(56, &this->segments_lock);
    }
    return v1;
}

long double HLSManager::getSegmentDuration(const int i_stream, const int i_segment) {
    HLSContent *v3; // eax@2
    //long double result; // fst7@3

    if ( this->b_ready )
    {
        v3 = this->p_hls_content;
        if ( v3 )
            return v3->getSegmentDuration(i_stream, i_segment);
    }
    return 0.0;
}

int HLSManager::getMaxSegmentID(const int i_stream) {
    HLSContent *v2; // eax@2

    if ( this->b_ready && (v2 = this->p_hls_content) != 0 )
        return v2->getMaxSegment(i_stream);
    return -1;
}

char* get_piece_key(const int i_stream, const int i_piece)
{
    char *psz_key; // [sp+2Ch] [bp-10h]@1

    psz_key = 0;
    asprintf(&psz_key, "%d_%d", i_stream, i_piece);
    return psz_key;
}

int hash_contains_key(const hash_t_0 *p_dict, const char *psz_key)
{
    signed int result; // eax@1
    int v3; // edx@2
    char v4; // al@3
    const char *v5; // esi@4
    unsigned int v6; // edi@4
    //int v7; // ebp@4
    //__int64 v8; // rax@5
    //__int64 v9; // rcx@5
    //int v10; // edi@5
    hash_entry_t_0 *v11; // esi@7

    result = 0;
    if ( p_dict->p_entries )
    {
        v3 = 0;
        if ( psz_key )
        {
            v4 = *psz_key;
            if ( *psz_key )
            {
                v5 = psz_key;
                v6 = 0;
                unsigned long long v7 = 0;
                do
                {
                    v7 = (v6+*v5)*0x401;
                    v6 = (v7 >> 8) ^ v7;
                    ++v5;
                } while ( *v5 );
                v3 = v6 % p_dict->i_size;//v3 = __PAIR__((unsigned int)v7, v6) % p_dict->i_size;
            }
        }
        result = 0;
        v11 = p_dict->p_entries[v3];
        if ( v11 )
        {
            while ( strcmp(psz_key, v11->psz_key) )
            {
                v11 = v11->p_next;
                if ( !v11 )
                    return 0;
            }
            result = 1;
        }
    }
    return result;
}

int HLSManager::isPieceRequestedOrReady(const int i_piece) {
    int v2; // ebx@2
    char *v3; // edi@3
    char v4; // ST18_1@3
    int i; // ebx@6
    piece_ready_t *v6; // eax@7
    signed int v7; // ebx@11

    if ( this->p_pieces_ready || (v7 = 0, this->p_piece_requests) )
    {
        v2 = 0;
        Mypthread_mutex_lock(57, &this->segments_lock);
        while ( v2 < this->p_hls_content->getStreamCount() )
        {
            v3 = get_piece_key(v2, i_piece);
            v4 = hash_contains_key(this->p_piece_requests, v3);
            free(v3);
            if ( v4 )
            {
                LABEL_12:
                v7 = 1;
                goto isPieceRequestedOrReady_return;
            }
            ++v2;
        }
        for ( i = 0; i < array_count(this->p_pieces_ready); ++i )
        {
            v6 = array_item_at_index(this->p_pieces_ready, i);
            if ( v6 && v6->i_piece == i_piece )
                goto LABEL_12;
        }
        v7 = 0;
        isPieceRequestedOrReady_return:
        Mypthread_mutex_unlock(57, &this->segments_lock);
    }
    return v7;
}

char ** hash_get_all_keys(const hash_t_0 *p_dict)
{
    hash_entry_t_0 **v1; // ebx@1
    int v2; // edx@3
    int v3; // ecx@3
    hash_entry_t_0 *i; // eax@4
    char** v5; // edi@7
    int v6; // esi@8
    int v7; // ebp@8
    hash_entry_t_0 *j; // ebx@9
    int v9; // esi@13
    int v11; // esi@15

    v1 = p_dict->p_entries;
    if ( v1 )
    {
        if ( p_dict->i_size <= 0 )
        {
            v9 = 0;
            v5 = (char**)malloc(sizeof(void*));
            goto LABEL_14;
        }
        v2 = 0;
        v3 = 0;
        do
        {
            for ( i = v1[v3]; i; ++v2 )
                i = i->p_next;
            ++v3;
        } while ( v3 != p_dict->i_size );
        v5 = (char**)malloc(sizeof(void*) * (v2 + 1));
    }
    else
    {
        v11 = p_dict->i_size;
        v5 = (char**)malloc(sizeof(void*));
        if ( v11 <= 0 )
        {
            v9 = 0;
            goto LABEL_14;
        }
    }
    v6 = 0;
    v7 = 0;
    while ( 1 )
    {
        for ( j = v1[v7]; j; ++v6 )
        {
            v5[v6] = strdup(j->psz_key);
            j = j->p_next;
        }
        if ( p_dict->i_size <= ++v7 )
            break;
        v1 = p_dict->p_entries;
    }
    v9 = v6;
    LABEL_14:
    v5[v9] = 0;
    return v5;
}

int hash_keys_count(const hash_t_0 *p_dict)
{
    int result; // eax@1
    hash_entry_t_0 **v2; // ebx@1
    int v3; // ecx@3
    hash_entry_t_0 *i; // edx@4

    result = 0;
    v2 = p_dict->p_entries;
    if ( v2 && p_dict->i_size > 0 )
    {
        v3 = 0;
        do
        {
            for ( i = v2[v3]; i; ++result )
                i = i->p_next;
            ++v3;
        }
        while ( v3 != p_dict->i_size );
    }
    return result;
}

array_t<quality_segmentid> *HLSManager::getSegmentsReadyAndRequested() {
    int v1; // ebx@2
    array_t<quality_segmentid> *v2; // edi@2
    quality_segmentid *v3; // ebp@3
    quality_segmentid* v4; // eax@4
    char **v5; // eax@7
    char **v6; // ebp@7
    unsigned int v7; // eax@9
    unsigned int v8; // ebx@9
    char **v9; // esi@10
    quality_segmentid* v10; // eax@11
    unsigned int v12; // [sp+14h] [bp-38h]@7
    int i_segment_id; // [sp+28h] [bp-24h]@7
    int i_segment_quality; // [sp+2Ch] [bp-20h]@7

    if ( this->p_segments_ready || this->p_segment_requests )
    {
        v1 = 0;
        v2 = array_new<quality_segmentid>();
        Mypthread_mutex_lock(58, &this->segments_lock);
        while ( v1 < array_count(this->p_segments_ready) )
        {
            v3 = array_item_at_index(this->p_segments_ready, v1);
            if ( v3 )
            {
                v4 = new quality_segmentid();
                v4->i_quality = v3->i_quality;
                v4->i_segment_id = v3->i_segment_id;
                array_append(v2, v4);
            }
            ++v1;
        }
        v12 = hash_keys_count(this->p_segment_requests);
        v5 = hash_get_all_keys(this->p_segment_requests);
        i_segment_id = 0;
        i_segment_quality = 0;
        v6 = v5;
        if ( v12 && v5 )
        {
            v7 = 0;
            v8 = 0;
            do
            {
                v9 = &v6[v7];
                if ( *v9 )
                {
                    sscanf(*v9, "%d_%d", &i_segment_quality, &i_segment_id);
                    v10 = new quality_segmentid();
                    v10->i_quality = i_segment_quality;
                    v10->i_segment_id = i_segment_id;
                    array_append(v2, v10);
                    free(*v9);
                }
                v7 = ++v8;
            } while ( v12 > v8 );
        }
        free(v6);
        Mypthread_mutex_unlock(58, &this->segments_lock);
    }
    else
    {
        v2 = 0;
    }
    return v2;
}

array_t<quality_segmentid> *HLSManager::getPiecesReadyAndRequested() {
    int v1; // ebx@2
    array_t<quality_segmentid> *v2; // edi@2
    piece_ready_t *v3; // ebp@3
    quality_segmentid* v4; // eax@4
    char **v5; // eax@7
    char **v6; // ebp@7
    unsigned int v7; // eax@9
    unsigned int v8; // ebx@9
    char **v9; // esi@10
    quality_segmentid* v10; // eax@11
    unsigned int v12; // [sp+14h] [bp-38h]@7
    int i_quality; // [sp+28h] [bp-24h]@7
    int i_piece_id; // [sp+2Ch] [bp-20h]@7

    if ( this->p_pieces_ready || this->p_piece_requests )
    {
        v1 = 0;
        v2 = array_new<quality_segmentid>();
        Mypthread_mutex_lock(59, &this->segments_lock);
        while ( v1 < array_count(this->p_pieces_ready) )
        {
            v3 = array_item_at_index(this->p_pieces_ready, v1);
            if ( v3 )
            {
                v4 = new quality_segmentid();
                v4->i_quality = v3->i_stream;
                v4->i_segment_id = v3->i_piece;
                array_append(v2, v4);
            }
            ++v1;
        }
        v12 = hash_keys_count(this->p_piece_requests);
        v5 = hash_get_all_keys(this->p_piece_requests);
        i_quality = 0;
        i_piece_id = 0;
        v6 = v5;
        if ( v12 && v5 )
        {
            v7 = 0;
            v8 = 0;
            do
            {
                v9 = &v6[v7];
                if ( *v9 )
                {
                    sscanf(*v9, "%d_%d", &i_quality, &i_piece_id);
                    v10 = new quality_segmentid();
                    v10->i_quality = i_quality;
                    v10->i_segment_id = i_piece_id;
                    array_append(v2, v10);
                    free(*v9);
                }
                v7 = ++v8;
            }
            while ( v12 > v8 );
        }
        free(v6);
        Mypthread_mutex_unlock(59, &this->segments_lock);
    }
    else
    {
        v2 = 0;
    }
    return v2;
}

void HLSManager::cleanSegment(const int i_stream, const int i_segment) {
    HLSContent *v3; // eax@2

    if ( this->b_ready )
    {
        v3 = this->p_hls_content;
        if ( v3 ) {
            v3->cleanSegmentData(i_stream, i_segment);
        }
    }
}

unsigned int HLSManager::downloadPiece(const int i_stream, const int i_piece, const int i_segment,
                                       uint64_t i_offset, uint64_t i_size) {
    unsigned int v6; // edi@1
    char *v7; // esi@1
    piece_request_info_t *p_dict; // edi@2
    //hash_t_0 *p_dict; // ST00_4@2
    stream_piece* v10; // eax@2

    v6 = -1;
    v7 = get_piece_key(i_stream, i_piece);
    if ( !hash_contains_key(this->p_piece_requests, v7) )
    {
        p_dict = new piece_request_info_t();
        p_dict->i_piece = i_piece;
        p_dict->i_stream = i_stream;
        //HIDWORD(v8->i_offset) = HIDWORD(i_offset);
        p_dict->i_segment = i_segment;
        //HIDWORD(v8->i_size) = HIDWORD(i_size);
        //LODWORD(v8->i_offset) = i_offset;
        p_dict->i_offset = i_offset;
        //LODWORD(v8->i_size) = i_size;
        p_dict->i_size = i_size;
        this->p_hls_content->ContentDownloaderControl(2, p_dict);
        //p_dict = (hash_t_0 *)v8;
        v6 = 0;
        free(p_dict);
        v10 = new stream_piece();
        v10->i_stream = i_stream;
        v10->i_piece = i_piece;
        hash_insert(this->p_piece_requests, v7, v10);
    }
    free(v7);
    return v6;
}

void hash_remove_value_for_key(const hash_t_0 *p_dict, const char *psz_key, void (*pf_free)(void *), void *p_obj)
{
    int v4; // esi@2
    char v5; // al@3
    const char *v6; // esi@4
    unsigned int v7; // edi@4
    //int64_t v8; // ebp@4
    //__int64 v9; // rax@5
    //__int64 v10; // rcx@5
    //int v11; // edi@5
    hash_entry_t_0 *v12; // edi@7
    hash_entry_t_0 *v13; // ebx@8
    __int64 v14; // [sp+8h] [bp-44h]@0
    int v15; // [sp+24h] [bp-28h]@2

    if ( p_dict->p_entries )
    {
        v4 = 0;
        v15 = p_dict->i_size;
        if ( psz_key )
        {
            v5 = *psz_key;
            if ( *psz_key )
            {
                v6 = psz_key;
                v7 = 0;
                unsigned long long v8 = 0;
                do
                {
                    v8 = (v7+*v6)*0x401;
                    v7 = (v8 >> 8) ^ v8;
                    v6++;
                } while ( *v6 );
                v14 = v15;
                v4 = v7 % v15;
            }
        }
        v12 = p_dict->p_entries[v4];
        if ( v12 )
        {
            v13 = 0;
            while ( strcmp(psz_key, v12->psz_key) )
            {
                if ( !v12->p_next )
                    return;
                v13 = v12;
                v12 = v12->p_next;
            }
            if ( pf_free )
                pf_free(v12->p_value);
            if ( v13 )
                v13->p_next = v12->p_next;
            else
                p_dict->p_entries[v4] = v12->p_next;
            free(v12->psz_key);
            free(v12);
        }
    }
}

void delete_hash_value(void *p_data)
{
    if ( p_data )
        free(p_data);
}

void HLSManager::notifySegmentReady(const int i_stream, const int i_segment, bool b_downloaded) {
    char *v4; // esi@2
    quality_segmentid *v5; // eax@3
    //quality_segmentid *v6; // ebp@3
    btSelfBitField *v7; // eax@3

    if ( b_downloaded )
    {
        v5 = new quality_segmentid();
        v5->i_quality = i_stream;
        //v6 = v5;
        v5->i_segment_id = i_segment;
        Mypthread_mutex_lock(60, &this->segments_lock);
        array_append(this->p_segments_ready, v5);
        Mypthread_mutex_unlock(60, &this->segments_lock);
        v7 = this->p_this->p_structure->BTCONTENT->p_hls_error;
        if ( v7 ) {
            v7->UnsetPiece(i_stream, i_segment);
        }
    }
    v4 = get_segment_key(i_stream, i_segment);
    hash_remove_value_for_key(this->p_segment_requests, v4, delete_hash_value, this->p_this);
    free(v4);
}

unsigned int HLSManager::downloadSegment(const int i_stream, const int i_segment) {
    char *v4; // ebx@1
    char v5; // al@1
    unsigned int v6; // edx@1
    piece_request_info_t *v7; // eax@2
    stream_segment* v9; // eax@2

    v4 = get_segment_key(i_stream, i_segment);
    v5 = hash_contains_key(this->p_segment_requests, v4);
    v6 = -1;
    if ( !v5 )
    {
        v7 = new piece_request_info_t();
        v7->i_stream = i_stream;
        v7->i_segment = i_segment;
        this->p_hls_content->ContentDownloaderControl(1, v7);
        free(v7);
        v9 = new stream_segment();
        v9->i_stream = i_stream;
        v9->i_segment = i_segment;
        hash_insert(this->p_segment_requests, v4, v9);
        v6 = 0;
    }
    free(v4);
    return v6;
}

void HLSManager::setReady(bool ready) {
    this->b_ready = ready;
}

void HLSManager::notifySegmentError(const int i_stream, const int i_segment) {
    char *v3; // ebp@1
    btSelfBitField *v4; // eax@1

    v3 = get_segment_key(i_stream, i_segment);
    hash_remove_value_for_key(this->p_segment_requests, v3, delete_hash_value, this->p_this);
    free(v3);
    v4 = this->p_this->p_structure->BTCONTENT->p_hls_error;
    if ( v4 ) {
        v4->SetPiece(i_stream, i_segment);
    }
}

hls_segment_t *HLSManager::getSegment(const int i_stream, const int i_segment) {
    HLSContent *v3; // eax@2
    hls_segment_t *result; // eax@3

    if ( this->b_ready && (v3 = this->p_hls_content) != 0 )
        result = v3->getSegmentData(i_stream, i_segment);
    else
        result = 0;
    return result;
}

void HLSManager::notifyPieceReady(piece_request_info_t_0 *p_request,
                                  piece_downloaded_info_t_0 *p_downloaded_info) {
    piece_ready_t *v3; // eax@2
    int64_t v6; // eax@2
    char *v7; // ebx@3

    if ( p_downloaded_info )
    {
        v3 = new piece_ready_t();
        //v4 = HIDWORD(p_downloaded_info->t_downloaded);
        v3->i_stream = p_request->i_stream;
        v3->i_piece = p_request->i_piece;
        v3->p_data = (block_t*)p_downloaded_info->p_data;
        v6 = p_downloaded_info->t_downloaded;
        //HIDWORD(v5->t_downloaded) = v4;
        v3->t_downloaded = v6;//LODWORD(v5->t_downloaded) = v6;
        Mypthread_mutex_lock(61, &this->segments_lock);
        array_append(this->p_pieces_ready, v3);
        Mypthread_mutex_unlock(61, &this->segments_lock);
    }
    v7 = get_piece_key(p_request->i_stream, p_request->i_piece);
    hash_remove_value_for_key(this->p_piece_requests, v7, delete_hash_value, this->p_this);
    free(v7);
}

HLSManager::~HLSManager() {
    this->b_ready = false;
    if ( this->p_hls_manifest_thread )
    {
        pthread_join(*this->p_hls_manifest_thread, 0);
        free(this->p_hls_manifest_thread);
        this->p_hls_manifest_thread = 0;
    }
    if ( this->p_hls_segment_thread )
    {
        pthread_join(*this->p_hls_segment_thread, 0);
        free(this->p_hls_segment_thread);
        this->p_hls_segment_thread = 0;
    }
    if ( this->p_hls_content )
    {
        delete this->p_hls_content;
    }
    hash_destroy(this->p_segment_requests, delete_hash_value, this->p_this);
    while ( array_count(this->p_segments_ready) > 0 )
    {
        free(array_item_at_index(this->p_segments_ready, 0));
        array_remove(this->p_segments_ready, 0);
    }
    array_destroy(this->p_segments_ready);
    hash_destroy(this->p_piece_requests, delete_hash_value, this->p_this);
    while ( array_count(this->p_pieces_ready) > 0 )
    {
        piece_ready_t* piece_ready = array_item_at_index(this->p_pieces_ready, 0);
        block_Release(piece_ready->p_data);
        free(piece_ready);
        array_remove(this->p_pieces_ready, 0);
    }
    array_destroy(this->p_pieces_ready);
    pthread_mutex_destroy(&this->segments_lock);
    pthread_mutex_destroy(&this->content_lock);
}

hls_segment_t * getSegment(const hls_stream_t *hls, const int index)
{
    int v2; // eax@2
    hls_segment_t *result; // eax@5

    if ( !hls )
        _assert_fail(
                "hls",
                "src/basetools/stream.cpp",
                0x184,
                "hls_segment_t* getSegment(const hls_stream_t*, int)");
    v2 = array_count(hls->p_segments);
    if ( v2 <= 0 || v2 <= index || (unsigned int)index >> 0x1F )
        result = 0;
    else
        result = array_item_at_index(hls->p_segments, index);
    return result;
}

generic_queue_t_0 *Init_Queue()
{
    generic_queue_t_0* v0 = new generic_queue_t_0();// *)malloc(0x20);
    v0->p_first = 0;
    v0->p_last = 0;
    pthread_mutex_init(&v0->lock, 0);
    return v0;
}

HLSContent::HLSContent(goalbit_t_0 *p_goalbit, HLSManager *p_manager) {
    //HLSParser *v3; // esi@1

    this->p_this = p_goalbit;
    this->p_parent = p_manager;
    this->p_parser = new HLSParser(this->p_this);
    this->p_content = 0;
    this->t_next_hls_update = 0LL;
    this->p_events_queue = Init_Queue();
    return;
}

void HLSContent::ManifestUploaderRun(void)
{
    int v1; // esi@5
    unsigned int v2; // edi@5
    int v3; // ebp@5
    unsigned int v4; // eax@6
    long double v5; // fst7@12
    //unsigned int v6; // edi@13
    //unsigned int v7; // esi@13
    //mtime_t v8; // kr00_8@13
    //float v10; // ST10_4@17
    int v12; // esi@20
    unsigned int v13; // edi@20
    int v14; // ebp@20
    unsigned int v15; // eax@21
    long double v16; // fst7@27
    //goalbit_structure_t *v17; // esi@29
    int64_t v18; // esi@29
    //unsigned int v19; // edi@29
    float v21; // ST10_4@30

    while ( this->p_this->p_structure->b_btc_alive )
    {
        /**
         * delay for starting
         */
        if (mdate() >= this->t_next_hls_update) {
            if (this->p_content == 0) {
                /**
                 * new hls_content_t(new stream, segment)
                 */
                if (loadHLSManifest(this->p_this->p_param->psz_hls_server_url) == 0) {
                    print_err(__func__, "[HLSContent] Can't load HLS Manifest!");
                    this->p_content = 0;
                    this->t_next_hls_update = mdate() + 5000000;
                    continue;
                }
                v12 = 0;
                v13 = 0;
                v14 = 0x80000001;
                /**
                 * v13 <- Max segment(last segment's i_sequence)
                 * v14 <- Stream number for max segment
                 */
                while (v12 < getStreamCount()) {
                    v15 = getMaxSegment(v12);
                    if (v15 && v13 < v15) {
                        v14 = v12;
                        v13 = v15;
                    }
                    ++v12;
                }
                /**
                 * set t_next_hls_update
                 */
                if (v13 && (v16 = getSegmentDuration(v14, v13), v16 != 0.0)) {
                    this->t_next_hls_update = mdate() + v16 * 1000000.0;
                } else {
                    this->t_next_hls_update = mdate() + 2000000;
                }
                Mypthread_mutex_lock(62, &this->p_this->p_structure->exec_lock);
                //v17 = this->p_this->p_structure;
                p_this->p_structure->i_hls_qualities = array_count(this->p_content->p_streams);
                Mypthread_mutex_unlock(62, &this->p_this->p_structure->exec_lock);
                v18 = this->t_next_hls_update;
                //v19 = HIDWORD(this->t_next_hls_update);
                print_dbg(__func__,
                          "[HLSContent] next HLS manifest reload in: %lld usecs",
                          v18 - mdate()
                );
            }
            /**
             * update stream
             */
            if (!updateHLSManifest()) {
                //goto LABEL_34;
                this->t_next_hls_update = mdate() + 2000000;
            } else {
                v1 = 0;
                v2 = 0;
                v3 = 0x80000001;
                while (v1 < getStreamCount()) {
                    v4 = getMaxSegment(v1);
                    if (v4 && v2 < v4) {
                        v3 = v1;
                        v2 = v4;
                    }
                    ++v1;
                }
                if (v2 && (v5 = getSegmentDuration(v3, v2), v5 != 0.0)) {
                    this->t_next_hls_update = (v5 * 1000000.0) + mdate();
                } else {
                    this->t_next_hls_update = mdate() + 2000000;
                }
            }
            print_dbg(__func__,
                      "[HLSContent] next HLS manifest reload in: %lld usecs",
                      this->t_next_hls_update - mdate()
            );
        }
        msleep(50000);
    }
}

void freeSegment(hls_segment_t *p_segment)
{
    if ( p_segment )
    {
        pthread_mutex_destroy(&p_segment->lock);
        if ( p_segment->psz_url )
            free(p_segment->psz_url);
        if ( p_segment->psz_desc )
            free(p_segment->psz_desc);
        if ( p_segment->psz_date )
            free(p_segment->psz_date);
        if ( p_segment->p_data )
            block_Release(p_segment->p_data);
        free(p_segment);
    }
}

void freeStream(hls_stream_t *p_hls)
{
    int v1; // ebx@2
    array_t<hls_segment_t> *v2; // eax@2
    hls_segment_t *v3; // eax@4

    if ( p_hls )
    {
        v1 = 0;
        pthread_mutex_destroy(&p_hls->lock);
        v2 = p_hls->p_segments;
        if ( v2 )
        {
            while ( v1 < array_count(v2) )
            {
                v3 = getSegment(p_hls, v1);
                if ( v3 )
                    freeSegment(v3);
                v2 = p_hls->p_segments;
                ++v1;
            }
            array_destroy(p_hls->p_segments);
        }
        if ( p_hls->psz_manifest_url )
            free(p_hls->psz_manifest_url);
        if ( p_hls->psz_codecs )
            free(p_hls->psz_codecs);
        if ( p_hls->psz_resolution )
            free(p_hls->psz_resolution);
        free(p_hls);
    }
}

void freeContent(hls_content_t *p_content)
{
    array_t<hls_stream_t> *v1; // eax@2
    int v2; // ebx@2
    hls_stream_t *v3; // eax@4

    if ( p_content )
    {
        v1 = p_content->p_streams;
        v2 = 0;
        if ( v1 )
        {
            while ( v2 < array_count(v1) )
            {
                v3 = getStream(p_content, v2);
                if ( v3 )
                    freeStream(v3);
                //v1 = p_content->p_streams;
                ++v2;
            }
            array_destroy(p_content->p_streams);
        }
        if ( p_content->psz_main_url )
            free(p_content->psz_main_url);
        free(p_content);
    }
}

int64_t getStreamSize(const hls_stream_t *p_hls)
{
    //unsigned int v1; // edi@1
    unsigned long long v2; // esi@1
    int v4; // ebx@4
    hls_segment_t *v5; // eax@7
    float v6; // ST2C_4@8
    long double v7; // fst7@8
    unsigned long long v8; // ST20_8@8
    //int64_t kkk;
    float v9; // ST2C_4@8
    long double v10; // fst6@8
    int count; // [sp+1Ch] [bp-30h]@3

    //v1 = 0;
    v2 = 0;
    if ( p_hls->i_bitrate )
    {
        count = array_count(p_hls->p_segments);
        if ( count > 0 )
        {
            v4 = 0;
            do
            {
                v5 = getSegment(p_hls, v4);
                if ( v5 )
                {
                    v6 = v2;//v6 = (long double)__PAIR__(v1, v2);
                    v7 = v6;
                    v8 = v5->i_bitrate >> 3;
                    //HIDWORD(v8) = HIDWORD(v5->i_bitrate) >> 3;
                    v9 = (long double)v8;
                    v10 = v7 + v9 * v5->f_duration;
                    if ( v10 < 9.223372e18 )
                    {
                        //v1 = (unsigned __int64)(signed __int64)v10 >> 0x20;
                        //v2 = (signed __int64)v10;
                        v2 = v10;
                    }
                    else
                    {
                        //v2 = (signed __int64)(v10 - 9.223372e18);
                        //v1 = ((unsigned __int64)(signed __int64)(v10 - 9.223372e18) >> 0x20) + 0x80000000;
                        v2 = v10-9.223372e18;
                    }
                }
                ++v4;
            }
            while ( v4 != count );
        }
    }
    return v2;
}

hls_segment_t * copySegment(const hls_segment_t *p_segment)
{
    hls_segment_t *v1; // esi@1
    hls_segment_t *v2; // eax@2
    //int v3; // edx@2
    int64_t v4; // eax@2
    //int v5; // edx@2
    int64_t v6; // eax@2
    //int v7; // edx@2
    int v8; // eax@2
    //int v9; // edx@2
    int64_t v10; // eax@2
    char *v11; // eax@2
    char *v12; // edx@2
    char *v13; // eax@2
    char *v14; // edx@4
    char *v15; // eax@4
    block_t *v16; // edx@6
    block_t *v17; // eax@6

    v1 = 0;
    if ( p_segment )
    {
        v2 = new hls_segment_t();
        //v3 = HIDWORD(p_segment->i_size);
        v1 = v2;
        v2->i_sequence = p_segment->i_sequence;
        v2->f_duration = p_segment->f_duration;
        v4 = p_segment->i_size;
        //HIDWORD(v1->i_size) = v3;
        //v5 = HIDWORD(p_segment->i_bitrate);
        v1->i_size = v4;//LODWORD(v1->i_size) = v4;
        v6 = p_segment->i_bitrate;
        //HIDWORD(v1->i_bitrate) = v5;
        //v7 = HIDWORD(p_segment->t_downloaded);
        v1->i_bitrate = v6;//LODWORD(v1->i_bitrate) = v6;
        v8 = p_segment->t_downloaded;
        //HIDWORD(v1->t_downloaded) = v7;
        //v9 = HIDWORD(p_segment->t_download);
        v1->t_downloaded = v8;//LODWORD(v1->t_downloaded) = v8;
        v10 = p_segment->t_download;
        //HIDWORD(v1->t_download) = v9;
        v1->t_download = v10;//LODWORD(v1->t_download) = v10;
        v1->b_discontinuity = p_segment->b_discontinuity;
        v11 = (char *)strdup(p_segment->psz_url);
        v12 = p_segment->psz_desc;
        v1->psz_url = v11;
        v13 = 0;
        if ( v12 )
            v13 = (char *)strdup(v12);
        v14 = p_segment->psz_date;
        v1->psz_desc = v13;
        v15 = 0;
        if ( v14 )
            v15 = (char *)strdup(v14);
        v16 = p_segment->p_data;
        v1->psz_date = v15;
        v17 = 0;
        if ( v16 )
            v17 = block_Duplicate(v16);
        v1->p_data = v17;
        pthread_mutex_init(&v1->lock, 0);
    }
    return v1;
}

int HLSContent::loadHLSManifest(const char *psz_manifest_url) {
    hls_content_t *v3; // eax@5
    signed int v5; // ebx@7
    //HLSManager *v7; // eax@11
    int v9; // ebp@15
    hls_stream_t *v10; // eax@16
    hls_stream_t *v11; // esi@16
    hls_stream_t *v12; // ebx@17
    HLSManager *v13; // eax@18
    int index; // [sp+18h] [bp-24h]@22
    int i_count; // [sp+1Ch] [bp-20h]@14

    /**
     * new hls_content_t
     */
    if ( this->p_parent ) {
        Mypthread_mutex_lock(63, &this->p_parent->content_lock);
    }
    if ( this->p_content )
    {
        freeContent(this->p_content);
        this->p_content = 0;
    }
    this->p_content = readMainManifest(psz_manifest_url);
    if ( this->p_parent )
    {
        Mypthread_mutex_unlock(63, &this->p_parent->content_lock);
        //v3 = this->p_content;
    }
    v5 = 0;

    /**
     * add segment array every stream if new stream array
     */
    if ( this->p_content )
    {
        if ( this->p_content->b_has_metafile )
        {
            i_count = array_count(this->p_content->p_streams);
            if ( i_count > 0 )
            {
                v9 = 0;
                do
                {
                    v10 = getStream(this->p_content, v9);
                    v11 = v10;
                    if ( v10 )
                    {
                        v12 = readStreamManifest(v10->psz_manifest_url);
                        if ( v12 )
                        {
                            v13 = this->p_parent;
                            if ( v13 ) {
                                Mypthread_mutex_lock(64, &v13->content_lock);
                            }
                            v11->f_target_duration = v12->f_target_duration;
                            v11->i_media_sequence = v12->i_media_sequence;
                            v11->i_version = v12->i_version;
                            v11->b_cache = v12->b_cache;
                            v11->b_end_list = v12->b_end_list;
                            if ( v11->b_end_list )
                                this->p_content->b_is_live = false;
                            index = 0;
                            v11->i_total_size = getStreamSize(v12);
                            while ( index < array_count(v12->p_segments) )
                            {
                                array_append(v11->p_segments, copySegment(getSegment(v12, index)));
                                ++index;
                            }
                            freeStream(v12);
                            if ( this->p_parent ) {
                                Mypthread_mutex_unlock(64, &this->p_parent->content_lock);
                            }
                        }
                    }
                    ++v9;
                } while ( v9 != i_count );
            }
        }
        /**
         * HLSManager set ready status
         * return 1
         */
        v5 = 1;
        if ( this->p_parent )
        {
            Mypthread_mutex_lock(65, &this->p_parent->content_lock);
            if ( this->p_parent )
            {
                this->p_parent->setReady(true);
            }
            Mypthread_mutex_unlock(65, &this->p_parent->content_lock);
        }
    }
    return v5;
}

block_t* sendHTTPRequest(goalbit_t_0 *p_goalbit, const char *psz_request_url, const int max_size, const int i_timeout)
{
    CURL* v4; // ebx@2
    CURLcode v5; // edi@2
    block_t *result; // eax@2
    const char* v7; // eax@4
    block_t *p_block; // [sp+1Ch] [bp-20h]@1

    print_dbg(__func__, "[HTTP] sendHTTPrequest - Incomming request: %s (timeout: %d)\n\n", psz_request_url, i_timeout);
    p_block = block_Alloc(max_size + 1);
    if ( p_block )
    {
        v4 = curl_easy_init();
        curl_easy_setopt(v4, CURLOPT_URL, psz_request_url);
        curl_easy_setopt(v4, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(v4, CURLOPT_WRITEFUNCTION, curl_write);
        curl_easy_setopt(v4, CURLOPT_WRITEDATA, p_block);
        curl_easy_setopt(v4, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(v4, CURLOPT_FAILONERROR, 1);
        curl_easy_setopt(v4, CURLOPT_TIMEOUT, i_timeout);
        v5 = curl_easy_perform(v4);
        curl_easy_cleanup(v4);
        result = p_block;
        if ( v5 ) {
            v7 = curl_easy_strerror(v5);
            print_dbg(__func__, "[HTTP] sendHTTPrequest - Error: %s (URL: %s)\n\n", v7, psz_request_url);
            block_Release(p_block);
            result = 0;
        } else {
            return result;
        }
    }
    else
    {
        print_err(__func__, "[HTTP] sendHTTPrequest - Not available memory for p_block\n\n");
        result = 0;
    }
    return result;
}

size_t curl_write(void *ptr, size_t size, size_t nmemb, block_t *p_result)
{
    size_t v4; // ebx@1
    size_t v5; // edx@2
    size_t v6; // eax@2
    uint8_t *v7; // eax@4

    v4 = 0;
    if ( !p_result )
        return v4;
    v4 = nmemb * size;
    v5 = p_result->i_buffer_index;
    v6 = v5 + nmemb * size;
    if ( (v6 & 0x80000000) == 0 )
    {
        if ( v6 > p_result->i_buffer_size )
        {
            v7 = (uint8_t *)realloc(p_result->p_buffer, v6 + 1);
            v5 = p_result->i_buffer_index;
            p_result->i_buffer_size = v4 + v5 + 1;
            p_result->p_buffer = v7;
        }
        else
        {
            v7 = p_result->p_buffer;
        }
        memcpy(&v7[v5], ptr, v4);
        p_result->i_buffer_index += v4;
        return v4;
    }
    print_err(__func__, "\n\nCan't continue downloading... max buffer size exceeded (%d)\n\n", 0x7FFFFFFF);
    return 0;
}

hls_content_t *HLSContent::readMainManifest(const char *psz_manifest_url) {
    mtime_t v2; // edi@4
    //const unsigned int *v3; // edx@5
    unsigned int v4; // ecx@5
    char *v5; // esp@5
    //unsigned int v6; // eax@5
    hls_content_t *v7; // ebx@5
    //int v8; // edx@6
    //char v10[0x10]; // [sp+23h] [bp-35h]@5
    //const unsigned int *v11; // [sp+28h] [bp-30h]@5
    int diff; // [sp+2Ch] [bp-2Ch]@5
    block_t *p_; // [sp+38h] [bp-20h]@4
    //int v14; // [sp+3Ch] [bp-1Ch]@1

    //v14 = *MK_FP(__GS__, 0x14);
    if ( !psz_manifest_url || !*psz_manifest_url || !this->p_parser ) {
        //goto LABEL_8;
        return 0;
    }
    v2 = mdate();
    p_ = sendHTTPRequest(this->p_this, psz_manifest_url, 0x19000, 0xA);
    if ( !p_ )
    {
        print_err(__func__, "[HLSContent] Can't read main manifest!");
        return 0;//goto LABEL_6;
    }
    diff = mdate() - v2;
    print_dbg(__func__,
              "[HLSContent] readMainManifest - url = %s, downloaded time = %f",
              psz_manifest_url,
              diff / 1000000.0
    );
    p_->p_buffer[p_->i_buffer_index] = 0;
    //*(_BYTE *)(p_[2] + p_[1]) = 0;
    //v3 = p_;
    v4 = p_->i_buffer_size;
    p_->i_buffer_index++;//++v3[1];
    v5 = (char*)alloca(v4 + 0x10);
    //v6 = v3[2];
    //v11 = v3;
    memcpy(v5, p_->p_buffer, v4);
    v5[p_->i_buffer_size] = 0;//*(_BYTE *)(((unsigned int)&v10 & 0xFFFFFFF0) + *v11) = 0;
    print_msg(__func__,
              "[HLSContent] Main Manifest (%s):\n\n%s\n\n",
              psz_manifest_url,
              v5);
    v7 = this->p_parser->parseManifest(v5, p_->i_buffer_size, psz_manifest_url);
    block_Release(p_);
    //LABEL_6:
    //v8 = *MK_FP(__GS__, 0x14) ^ v14;
    return v7;
}

queue_node_t* Pop_Value(generic_queue_t_0 *queue)
{
    queue_node_t *v1; // edi@1
    queue_node_t *v2; // esi@2

    v1 = 0;
    if ( queue )
    {
        Mypthread_mutex_lock(-1, &queue->lock);
        v2 = queue->p_first;
        if ( v2 )
        {
            if ( v2 == queue->p_last )
                queue->p_last = 0;
            v1 = v2;
            queue->p_first = v2->p_next;
        }
        Mypthread_mutex_unlock(-1, &queue->lock);
    }
    return v1;
}

void HLSContent::ContentDownloaderRun() {
    queue_node_t *v5; // eax@12
    piece_request_info_t* v8; // edi@15

    while ( this->p_this->p_structure->b_btc_alive ) {
        if ( this->p_parent->isReady() ) {
            queue_node_t *v1 = Pop_Value(this->p_events_queue);
            if ( v1 ) {
                if (v1->event.i_event == 1) {
                    v8 = v1->event.value.piece_request_info;
                    downloadHLSSegment(v8->i_stream, v8->i_segment);
                    free(v8);
                } else if (v1->event.i_event == 2) {
                    downloadHLSPiece(v1->event.value.piece_request_info);
                    free(v1->event.value.piece_request_info);
                }
                free(v1);
                continue;
            }
        }
        msleep(50000);
    }

    while (v5 = Pop_Value(this->p_events_queue)) {
        if (v5->event.i_event == 1 || v5->event.i_event == 2)
            free(v5->event.value.piece_request_info);
        free(v5);
    }
    return;
}

bool HLSContent::isLiveContent() {
    bool v1 = false; // esi@1
    Mypthread_mutex_lock(67, &this->p_parent->content_lock);
    if ( this->p_content ) {
        v1 = this->p_content->b_is_live;
    }
    Mypthread_mutex_unlock(67, &this->p_parent->content_lock);
    return v1;
}

goalbit_qualities_desc_t_0* HLSContent::exportStreamsDescription() {
    unsigned int v2; // ebx@2
    hls_stream_t *v3; // eax@5

    goalbit_qualities_desc_t_0 *v1 = new goalbit_qualities_desc_t_0();
    Mypthread_mutex_lock(68, &this->p_parent->content_lock);
    int i_quality_num = array_count(this->p_content->p_streams);
    v1->i_quality_num = i_quality_num;
    v1->p_qualities = new goalbit_quality_info_t_0[0x10];
    if ( i_quality_num > 0 )
    {
        v2 = 0;
        do {
            v3 = getStream(this->p_content, v2);
            if (v3) {
                v1->p_qualities[v2].i_quality = v2;
                v1->p_qualities[v2].i_bitrate = v3->i_bitrate;
                if (v3->psz_resolution && *v3->psz_resolution) {
                    v1->p_qualities[v2].psz_resolution = strdup(v3->psz_resolution);
                } else {
                    v1->p_qualities[v2].psz_resolution = 0;
                }
            }
        } while (++v2 < i_quality_num);
    }
    Mypthread_mutex_unlock(68, &this->p_parent->content_lock);
    return v1;
}

int HLSContent::getStreamCount() {
    HLSManager *v1; // eax@1
    hls_content_t *v2; // eax@3
    int v3; // esi@3
    HLSManager *v4; // eax@5

    v1 = this->p_parent;
    if ( v1 )
        Mypthread_mutex_lock(69, &v1->content_lock);
    v2 = this->p_content;
    v3 = 0;
    if ( v2 )
        v3 = array_count(v2->p_streams);
    v4 = this->p_parent;
    if ( v4 )
        Mypthread_mutex_unlock(69, &v4->content_lock);
    return v3;
}

int HLSContent::getMinSegment(const int i_current_stream) {
    int v3; // edi@3
    const hls_stream_t *v5; // esi@3
    hls_segment_t *v6; // eax@5

    if ( this->p_parent )
        Mypthread_mutex_lock(70, &this->p_parent->content_lock);
    v3 = 0;
    v5 = getStream(this->p_content, i_current_stream);
    if ( v5 )
    {
        if ( array_count(v5->p_segments) > 0 )
        {
            v6 = getSegment(v5, 0);
            if ( v6 )
                v3 = v6->i_sequence;
        }
    }
    if ( this->p_parent ) {
        Mypthread_mutex_unlock(70, &this->p_parent->content_lock);
    }
    return v3;
}

int HLSContent::getMaxSegment(const int i_current_stream) {
    HLSManager *v2; // eax@1
    int v3; // edi@3
    hls_stream_t *v4; // eax@3
    const hls_stream_t *v5; // esi@3
    int v6; // eax@4
    hls_segment_t *v7; // eax@5
    HLSManager *v8; // eax@7

    v2 = this->p_parent;
    if ( v2 )
        Mypthread_mutex_lock(71, &v2->content_lock);
    v3 = 0;
    v4 = getStream(this->p_content, i_current_stream);
    v5 = v4;
    if ( v4 )
    {
        v6 = array_count(v4->p_segments);
        if ( v6 > 0 )
        {
            v7 = getSegment(v5, v6 - 1);
            if ( v7 )
                v3 = v7->i_sequence;
        }
    }
    v8 = this->p_parent;
    if ( v8 )
        Mypthread_mutex_unlock(71, &v8->content_lock);
    return v3;
}

hls_segment_t *findSegment(const hls_stream_t *hls, const int sequence)
{
    int v2; // edi@2
    hls_segment_t *result; // eax@2
    int v4; // ebx@3

    if ( !hls )
        _assert_fail(
                "hls",
                "src/basetools/stream.cpp",
                0x193,
                "hls_segment_t* findSegment(const hls_stream_t*, int)");
    v2 = array_count(hls->p_segments);
    result = 0;
    if ( v2 > 0 )
    {
        v4 = 0;
        while ( 1 )
        {
            result = getSegment(hls, v4);
            if ( result )
            {
                if ( result->i_sequence == sequence )
                    break;
            }
            if ( ++v4 == v2 )
                return 0;
        }
    }
    return result;
}

hls_segment_t *HLSContent::getSegmentData(const int i_current_stream, const int i_chosen_sequence) {
    HLSManager *v3; // eax@1
    hls_segment_t *v4; // esi@3
    const hls_stream_t *v5; // eax@3
    const hls_segment_t *v6; // eax@4
    HLSManager *v7; // eax@6

    v3 = this->p_parent;
    if ( v3 )
        Mypthread_mutex_lock(72, &v3->content_lock);
    v4 = 0;
    v5 = getStream(this->p_content, i_current_stream);
    if ( v5 )
    {
        v6 = findSegment(v5, i_chosen_sequence);
        if ( v6 )
            v4 = copySegment(v6);
    }
    v7 = this->p_parent;
    if ( v7 )
        Mypthread_mutex_unlock(72, &v7->content_lock);
    return v4;
}

long double HLSContent::getSegmentDuration(const int i_current_stream, const int i_chosen_sequence) {
    HLSManager *v3; // eax@1
    const hls_stream_t *v4; // eax@3
    hls_segment_t *v5; // eax@4
    HLSManager *v6; // eax@6
    float f_result; // [sp+1Ch] [bp-10h]@3

    v3 = this->p_parent;
    if ( v3 )
        Mypthread_mutex_lock(73, &v3->content_lock);
    v4 = getStream(this->p_content, i_current_stream);
    f_result = 0.0;
    if ( v4 )
    {
        v5 = findSegment(v4, i_chosen_sequence);
        if ( v5 )
            f_result = v5->f_duration;
    }
    v6 = this->p_parent;
    if ( v6 )
        Mypthread_mutex_unlock(73, &v6->content_lock);
    return f_result;
}

void HLSContent::cleanSegmentData(const int i_current_stream, const int i_chosen_sequence) {
    HLSManager *v3; // eax@1
    const hls_stream_t *v4; // eax@3
    hls_segment_t *v5; // eax@4
    hls_segment_t *v6; // esi@4
    HLSManager *v7; // eax@7

    v3 = this->p_parent;
    if ( v3 )
        Mypthread_mutex_lock(74, &v3->content_lock);
    v4 = getStream(this->p_content, i_current_stream);
    if ( v4 )
    {
        v5 = findSegment(v4, i_chosen_sequence);
        v6 = v5;
        if ( v5 )
        {
            if ( v5->p_data )
            {
                block_Release(v5->p_data);
                v6->p_data = 0;
            }
        }
    }
    v7 = this->p_parent;
    if ( v7 )
        Mypthread_mutex_unlock(74, &v7->content_lock);
}

void HLSContent::ContentDownloaderControl(const int i_action, piece_request_info_t *p_data) {
    queue_node_t *v3; // ebp@7
    char v4 = 0; // di@8
    piece_request_info_t_0 *v5; // eax@11
    queue_node_t *v6; // ebp@14
    char v7 = 0; // di@15
    piece_request_info_t* v8; // eax@18
    int v9; // edx@18
    int v10; // eax@19
    piece_request_info_t *v11; // eax@22
    generic_event_t_0 ST04_12_22; // ST04_12@22
    piece_request_info_t *v13; // eax@24
    generic_event_t_0 ST04_12_24; // ST04_12@24
    //unsigned int event_8; // [sp+20h] [bp-2Ch]@0
    //unsigned int v16; // [sp+2Ch] [bp-20h]@0

    if ( !p_data || !this->p_events_queue || !this->p_parent->isReady() )
        return;

    if ( i_action == 1 )
    {
        Mypthread_mutex_lock(75, &this->p_events_queue->lock);
        v6 = this->p_events_queue->p_first;
        if ( v6 )
        {
            v7 = 0;
            do
            {
                if ( v6->event.i_event == 1 )
                {
                    v8 = v6->event.value.piece_request_info;
                    v9 = v8->i_segment;
                    if ( v9 == p_data->i_segment )
                    {
                        v10 = v8->i_stream;
                        v7 = 1;
                        if ( v10 != p_data->i_stream )
                        {
                            this->p_parent->notifySegmentReady(v10, v9, false);
                            v6->event.value.piece_request_info = p_data;
                        }
                    }
                }
                v6 = v6->p_next;
            } while ( v6 );
        }
        Mypthread_mutex_unlock(75, &this->p_events_queue->lock);
        if ( v7 ) {
            return;
        }
        v11 = new piece_request_info_t();
        v11->i_stream = p_data->i_stream;
        v11->i_piece = p_data->i_piece;
        v11->i_segment = p_data->i_segment;
        v11->i_offset = p_data->i_offset;
        ST04_12_22.value.piece_request_info = v11;
        ST04_12_22.i_event = 1;
        Push_Value(this->p_events_queue, ST04_12_22);
        return;
    }
    if ( i_action == 2 ) {
        Mypthread_mutex_lock(76, &this->p_events_queue->lock);
        v3 = this->p_events_queue->p_first;
        if (v3) {
            v4 = 0;
            do {
                if (v3->event.i_event == 2) {
                    v5 = v3->event.value.piece_request_info;
                    if (v5->i_piece == p_data->i_piece) {
                        v4 = 1;
                        if (v5->i_stream != p_data->i_stream) {
                            this->p_parent->notifyPieceReady(v5, 0);
                            v3->event.value.i_int = p_data->i_stream;
                        }
                    }
                }
                v3 = v3->p_next;
            } while (v3);
        }
        Mypthread_mutex_unlock(76, &this->p_events_queue->lock);
        if (v4) {
            return;
        }
        v13 = new piece_request_info_t();
        v13->i_stream = p_data->i_stream;
        v13->i_piece = p_data->i_piece;
        v13->i_segment = p_data->i_segment;
        v13->i_offset = p_data->i_offset;
        v13->i_size = p_data->i_size;
        ST04_12_24.value.piece_request_info = v13;
        ST04_12_24.i_event = 2;
        Push_Value(this->p_events_queue, ST04_12_24);
        return;
    }
    return;
}

void mapStream(const hls_stream_t *p_stream_src, hls_stream_t *p_stream_dst)
{
    //int v2; // edx@1
    long long v3; // eax@1
    //uint64_t v4; // rax@1
    char *v5; // eax@7
    char *v6; // edx@7
    char *v7; // eax@7
    char *v8; // edx@9
    char *v9; // eax@9

    //v2 = HIDWORD(p_stream_src->i_bitrate);
    p_stream_dst->i_program_id = p_stream_src->i_program_id;
    v3 = p_stream_src->i_bitrate;
    //HIDWORD(p_stream_dst->i_bitrate) = v2;
    //HIDWORD(v4) = HIDWORD(p_stream_src->i_total_size);
    p_stream_dst->i_bitrate = v3;//LODWORD(p_stream_dst->i_bitrate) = v3;
    p_stream_dst->f_target_duration = p_stream_src->f_target_duration;
    //LODWORD(v4) = p_stream_src->i_total_size;
    p_stream_dst->i_total_size = p_stream_src->i_total_size;//p_stream_dst->i_total_size = v4;
    p_stream_dst->i_media_sequence = p_stream_src->i_media_sequence;
    p_stream_dst->i_version = p_stream_src->i_version;
    p_stream_dst->b_cache = p_stream_src->b_cache;
    p_stream_dst->b_end_list = p_stream_src->b_end_list;
    if ( p_stream_dst->psz_manifest_url )
        free(p_stream_dst->psz_manifest_url);
    if ( p_stream_dst->psz_codecs )
        free(p_stream_dst->psz_codecs);
    if ( p_stream_dst->psz_resolution )
        free(p_stream_dst->psz_resolution);
    v5 = (char *)strdup(p_stream_src->psz_manifest_url);
    v6 = p_stream_src->psz_codecs;
    p_stream_dst->psz_manifest_url = v5;
    v7 = 0;
    if ( v6 )
        v7 = (char *)strdup(v6);
    v8 = p_stream_src->psz_resolution;
    p_stream_dst->psz_codecs = v7;
    v9 = 0;
    if ( v8 )
        v9 = (char *)strdup(v8);
    p_stream_dst->psz_resolution = v9;
}

hls_stream_t * mergeStreams(const hls_stream_t *p_old_stream, const hls_stream_t *p_new_stream, const size_t i_min_valid_sequence)
{
    hls_stream_t *v3; // eax@4
    int v4; // ebx@5
    const hls_segment_t *v5; // eax@8
    hls_segment_t *v6; // eax@10
    int i; // ebx@12
    hls_segment_t *v9; // eax@15
    const hls_segment_t *v10; // ebp@15
    hls_segment_t *v11; // eax@18
    hls_stream_t *p_stream; // [sp+1Ch] [bp-20h]@3

    if ( !p_old_stream )
    {
        p_old_stream = p_new_stream;
        return copyStream(p_old_stream);
    }
    if ( !p_new_stream )
        return copyStream(p_old_stream);
    p_stream = 0;
    if ( p_old_stream->i_program_id == p_new_stream->i_program_id )
    {
        v3 = newStream(p_old_stream->i_program_id, p_new_stream->i_bitrate, p_new_stream->psz_manifest_url);
        p_stream = v3;
        if ( v3 )
        {
            v4 = 0;
            mapStream(p_new_stream, v3);
            while ( v4 < array_count(p_old_stream->p_segments) )
            {
                v5 = getSegment(p_old_stream, v4);
                if ( v5 )
                {
                    if ( v5->i_sequence >= i_min_valid_sequence )
                    {
                        v6 = copySegment(v5);
                        array_append(p_stream->p_segments, v6);
                    }
                }
                ++v4;
            }
            for ( i = 0; i < array_count(p_new_stream->p_segments); ++i )
            {
                v9 = getSegment(p_new_stream, i);
                v10 = v9;
                if ( v9 && !findSegment(p_old_stream, v9->i_sequence) && i_min_valid_sequence <= v10->i_sequence )
                {
                    v11 = copySegment(v10);
                    array_append(p_stream->p_segments, v11);
                }
            }
        }
    }
    return p_stream;
}

template <typename T>
void array_set_item_at_index(array_t<T> *p_array, T *p_elem, int i_index)
{
    p_array->pp_elems[i_index] = p_elem;
}

int HLSContent::updateHLSManifest() {
    //goalbit_structure_t *v1; // eax@1
    size_t v2; // eax@1
    int v3; // edi@4
    hls_stream_t *v5; // ebx@5
    hls_stream_t *v6; // eax@6
    HLSManager *v8; // eax@7
    hls_stream_t *v9; // eax@9
    HLSManager *v10; // eax@9
    int i_count; // [sp+18h] [bp-24h]@3
    size_t i_win_offset; // [sp+1Ch] [bp-20h]@1
    size_t i_win_offseta; // [sp+1Ch] [bp-20h]@3

    Mypthread_mutex_lock(77, &this->p_this->p_structure->exec_lock);
    i_win_offset = p_this->p_structure->i_hls_win_offset;
    Mypthread_mutex_unlock(77, &p_this->p_structure->exec_lock);

    v2 = 0;
    if ( i_win_offset != 0x80000001 )
        v2 = i_win_offset;
    i_win_offseta = v2;
    i_count = array_count(this->p_content->p_streams);
    if ( i_count > 0 )
    {
        v3 = 0;
        do
        {
            v5 = getStream(this->p_content, v3);
            if ( v5 )
            {
                v6 = readStreamManifest(v5->psz_manifest_url);
                if ( v6 )
                {
                    v5->f_target_duration = v6->f_target_duration;
                    v5->i_media_sequence = v6->i_media_sequence;
                    v5->i_version = v6->i_version;
                    v5->b_cache = v6->b_cache;
                    v5->b_end_list = v6->b_end_list;
                    mapStream(v5, v6);
                    v8 = this->p_parent;
                    if ( v8 )
                        Mypthread_mutex_lock(78, &v8->content_lock);
                    v9 = mergeStreams(v5, v6, i_win_offseta);
                    array_set_item_at_index(this->p_content->p_streams, v9, v3);
                    v10 = this->p_parent;
                    if ( v10 )
                        Mypthread_mutex_unlock(78, &v10->content_lock);
                    freeStream(v6);
                    freeStream(v5);
                }
            }
            ++v3;
        } while ( v3 != i_count );
    }
    return 1;
}

hls_stream_t *HLSContent::readStreamManifest(const char *psz_manifest_url) {
    hls_stream_t *v2; // edi@1
    int v3; // ebp@3
    int v4; // eax@4
    block_t *v5; // edi@4
    size_t v6; // eax@4
    int n; // ST28_4@4
    char *v8; // ebp@4
    hls_content_t *v9; // edx@4
    int v10; // eax@4
    block_t *p_block; // [sp+3Ch] [bp-20h]@3

    v2 = 0;
    if ( psz_manifest_url )
    {
        if ( *psz_manifest_url )
        {
            v3 = mdate();
            p_block = sendHTTPRequest(this->p_this, psz_manifest_url, 0x19000, 0xA);
            if ( p_block )
            {
                v4 = mdate();
                print_dbg(__func__, "[HLSContent] readStreamManifest - url = %s, downloaded time = %f",
                          psz_manifest_url,
                          (v4 - v3) / 1000000.0
                );
                p_block->p_buffer[p_block->i_buffer_index] = 0;
                v5 = p_block;
                v6 = p_block->i_buffer_index;
                p_block->i_buffer_index = v6 + 1;
                n = v6 + 1;
                v8 = (char *)malloc(v6 + 2);
                strncpy(v8, (const char *)v5->p_buffer, n);
                v8[v5->i_buffer_index] = 0;
                print_msg(__func__, "[HLSContent] Stream Manifest (%s):\n\n%s\n\n", psz_manifest_url, v8);
                v9 = this->p_content;
                v10 = 1;
                if ( v9 ) {
                    v10 = v9->i_version;
                }
                v2 = this->p_parser->parsePlaylist(v8, p_block->i_buffer_index, v10, psz_manifest_url);
                block_Release(p_block);
                free(v8);
            }
        }
    }
    return v2;
}

/**
 * download one segment
 * setting p_segments_ready or p_hls_error
 */
int HLSContent::downloadHLSSegment(const int i_current_stream, const int i_chosen_sequence) {
    //goalbit_structure_t *v3; // eax@1
    //goalbit_structure_t *v6; // eax@6
    unsigned int v7; // ebp@6
    hls_segment_t *v9; // eax@9
    segment_download_info_t_0 download_info; // [sp+2Ch] [bp-30h]@7

    //v3 = this->p_this->p_structure;
    if ( p_this->p_structure->BTCONTENT->p_hls_bitfield )
    {
        if ( p_this->p_structure->BTCONTENT->p_hls_bitfield->IsSet(i_current_stream, i_chosen_sequence) )
        {
            this->p_parent->notifySegmentReady(i_current_stream, i_chosen_sequence, false);
            return 0;
        }
        //v3 = this->p_this->p_structure;
    }
    Mypthread_mutex_lock(79, &p_this->p_structure->exec_lock);
    //v6 = this->p_this->p_structure;
    v7 = p_this->p_structure->exec_info.i_segment_id;
    Mypthread_mutex_unlock(79, &p_this->p_structure->exec_lock);
    if ( i_chosen_sequence < v7 ) {
        this->p_parent->notifySegmentReady(i_current_stream, i_chosen_sequence, false);
        return 0;
    }
    if ( getSegmentDownloadInfo(i_current_stream, i_chosen_sequence, &download_info) )
    {
        if ( download_info.b_is_downloaded )
        {
            free(download_info.psz_url);
            this->p_parent->notifySegmentReady(i_current_stream, i_chosen_sequence, true);
            return 1;
        }
        print_msg(__func__, "[HLSContent] Downloading segment %d for quality %d", i_chosen_sequence, i_current_stream);
        v9 = readSegment(
                download_info.psz_url,
                (signed int)(download_info.f_duration * download_info.i_bitrate * 1.25),
                download_info.f_duration);
        if ( v9 )
        {
            setSegmentData(i_current_stream, i_chosen_sequence, v9);
            freeSegment(v9);
            this->p_parent->notifySegmentReady(i_current_stream, i_chosen_sequence, true);
            free(download_info.psz_url);
            return 1;
        }
        free(download_info.psz_url);
    }
    this->p_parent->notifySegmentError(i_current_stream, i_chosen_sequence);
    return 0;
}

hls_segment_t *
HLSContent::readSegment(const char *psz_segment_url, const int i_max_size, const float f_duration) {
    hls_segment_t *v5; // ebp@1
    //long double v6; // fst6@1
    int v7; // ebx@2
    mtime_t v8; // rax@4
    //unsigned int v9; // edi@4
    //uint64_t v10; // esi@4
    mtime_t v11; // rax@4
    //mtime_t v12; // rcx@4
    //unsigned int v13; // edx@5
    long double v14; // kr00_4@5
    size_t v15; // esi@5
    //long double v16; // fst5@6
    //long double v18; // fst6@7
    //hls_segment_t *id; // ST00_4@14
    long double v23; // [sp+38h] [bp-84h]@7
    block_t *p_data; // [sp+68h] [bp-54h]@4
    //bool v25; // [sp+6Ch] [bp-50h]@2
    //long double v26; // [sp+70h] [bp-4Ch]@5

    v5 = newSegment(0, 0.0, 0LL, 0LL, psz_segment_url, 0, 0, 0);
    //v6 = f_duration;
    if ( v5 )
    {
        v7 = 0xA;
        //v25 = v6 > 0;
        if ( f_duration > 0 )
        {
            //            _FST6 = v6 + v6;
            //            __asm { frndint }
            //            v7 = (signed int)(double)_FST6;
            v7 = f_duration + f_duration;
        }
        v8 = mdate();
        //v9 = HIDWORD(v8);
        //v10 = v8;
        p_data = sendHTTPRequest(this->p_this, v5->psz_url, i_max_size, v7);
        v11 = mdate();
        //v12 = v11;
        if ( p_data )
        {
            v14 = v11 - v8;
            //HIDWORD(v11) = (v11 - __PAIR__(v9, v10)) >> 0x20;
            //v26 = v11 - v8;////v26 = __PAIR__(HIDWORD(v11), (unsigned int)v11 - v10);
            v15 = p_data->i_buffer_index;
            v5->f_duration = f_duration;
            v5->p_data = p_data;
            //HIDWORD(v5->i_size) = 0;
            v5->i_size = v15;//LODWORD(v5->i_size) = v15;
            if ( f_duration > 0 )
            {
                v23 = (double)v15 / f_duration;
                if ( v23 >= 9.223372e18 )
                {
                    v23 = v23 - 9.223372e18;//v23 = (signed __int64)(v21 - 9.223372e18);
                    //v18 = v14;//v18 = (long double)(signed __int64)__PAIR__(v13, v14);
                    //HIDWORD(v23) += 0x80000000;
                }
                else
                {
                    //v23 = v21;//v23 = (signed __int64)v21;
                    //v18 = v14;//v18 = (long double)(signed __int64)__PAIR__(v13, v14);
                }
            }
            else
            {
                //v16 = (long double)v14;
                v23 = (double)v15 / (v14 / 1000000.0);
                if ( v23 >= 9.223372e18 )
                {
                    //v18 = v14;
                    v23 = v23 - 9.223372e18;//v23 = (signed __int64)(v17 - 9.223372e18);
                    //HIDWORD(v23) += 0x80000000;
                }
                else
                {
                    //v18 = v14;
                    //v23 = v17;//v23 = (signed __int64)v17;
                }
            }
            v5->t_downloaded = v11;//LODWORD(v5->t_downloaded) = v12;
            //HIDWORD(v5->i_bitrate) = HIDWORD(v23);
            //HIDWORD(v5->t_downloaded) = HIDWORD(v12);
            v5->i_bitrate = v23;//LODWORD(v5->i_bitrate) = v23;
            //HIDWORD(v5->t_download) = v13;
            v5->t_download = v14;//LODWORD(v5->t_download) = v14;
            print_dbg(__func__,
                      "[HLSContent] readSegment - url = %s, max_size = %d, size = %lld, duration = %f, downloaded time = %f, bitrate = %d",
                      psz_segment_url,
                      i_max_size,
                      v15,
                      f_duration,
                      v14 / 1000000.0,
                      v23);
            return v5;
        }
        freeSegment(v5);
    }
    return 0;
}

block_t * sendHTTPRangeRequest(goalbit_t_0 *p_goalbit, const char *psz_request_url, const uint64_t i_offset, const uint64_t i_size, const int i_timeout)
{
    unsigned long long v5; // kr00_8@1
    CURL* v6; // esi@2
    CURLcode v7; // ebx@2
    block_t *result; // eax@2
    const char * v9; // eax@4
    //unsigned int v11; // [sp+24h] [bp-38h]@1
    //int v12; // [sp+28h] [bp-34h]@1
    //int v13; // [sp+2Ch] [bp-30h]@1
    block_t *p_block; // [sp+38h] [bp-24h]@1
    char *psz_range; // [sp+3Ch] [bp-20h]@2

    v5 = i_size + i_offset - 1;
    print_dbg(__func__,
              "[HTTP] sendHTTPRangeRequest - Incomming request: %s [%llu-%llu] (timeout: %d)\n\n",
              psz_request_url,
              i_offset,
              v5,
              i_timeout
    );
    p_block = block_Alloc(i_size);
    if ( p_block ) {
        psz_range = 0;
        asprintf(&psz_range, "%llu-%llu", i_offset, v5);
        v6 = curl_easy_init();
        curl_easy_setopt(v6, CURLOPT_URL, psz_request_url);
        curl_easy_setopt(v6, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(v6, CURLOPT_RANGE, psz_range);
        curl_easy_setopt(v6, CURLOPT_WRITEFUNCTION, curl_write);
        curl_easy_setopt(v6, CURLOPT_WRITEDATA, p_block);
        curl_easy_setopt(v6, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(v6, CURLOPT_FAILONERROR, 1);
        curl_easy_setopt(v6, CURLOPT_TIMEOUT, i_timeout);
        v7 = curl_easy_perform(v6);
        curl_easy_cleanup(v6);
        free(psz_range);
        result = p_block;
        if ( v7 ) {
            v9 = curl_easy_strerror(v7);
            print_dbg(__func__,
                      "[HTTP] sendHTTPRangeRequest - Error: %s (URL: %s [%llu-%llu])\n\n",
                      v9,
                      psz_request_url,
                      i_offset,
                      v5
            );
            block_Release(p_block);
            result = 0;
        }
    }
    else
    {
        print_err(__func__, "[HTTP] sendHTTPRangeRequest - Not available memory for p_block\n\n");
        result = 0;
    }
    return result;
}

int HLSContent::downloadHLSPiece(piece_request_info_t_0 *p_request) {
    //goalbit_structure_t *v2; // eax@1
    signed int result; // eax@4
    //goalbit_structure_t *v4; // eax@7
    unsigned int v5; // esi@7
    mtime_t v7; // kr00_8@10
    mtime_t v8; // rcx@10
    long long v10; // rcx@16
    //unsigned int v18; // [sp+48h] [bp-64h]@1
    segment_download_info_t_0 download_info; // [sp+68h] [bp-44h]@8
    piece_downloaded_info_t_0 downloaded_info; // [sp+7Ch] [bp-30h]@16
    block_t *p_data; // [sp+88h] [bp-24h]@10
    __int16 v22; // [sp+8Ch] [bp-20h]@10
    __int16 v23; // [sp+8Eh] [bp-1Eh]@10

    //v18 = HIDWORD(p_request->i_size);
    //v2 = this->p_this->p_structure;
    if ( p_this->p_structure->BTCONTENT->p_p2p_bitfield )
    {
        if ( p_this->p_structure->BTCONTENT->p_p2p_bitfield->IsSet(p_request->i_stream, p_request->i_piece) )
        {
            print_dbg(__func__,
                      "[HLSContent] downloadHLSPiece - Piece %d-%d (from segment %d) has been already downloaded",
                      p_request->i_stream,
                      p_request->i_piece,
                      p_request->i_segment);
            //LABEL_4:
            this->p_parent->notifyPieceReady(p_request, 0);
            return 0;
        }
        //v2 = this->p_this->p_structure;
    }
    Mypthread_mutex_lock(80, &p_this->p_structure->exec_lock);
    //v4 = this->p_this->p_structure;
    v5 = p_this->p_structure->exec_info.i_segment_id;
    Mypthread_mutex_unlock(80, &p_this->p_structure->exec_lock);
    if ( v5 > p_request->i_segment )
    {
        print_dbg(__func__,
                  "[HLSContent] downloadHLSPiece - Piece %d-%d (from segment %d) has expired",
                  p_request->i_stream,
                  p_request->i_piece,
                  p_request->i_segment);
        //goto LABEL_4;
        this->p_parent->notifyPieceReady(p_request, 0);
        return 0;
    }
    if ( !getSegmentDownloadInfo(p_request->i_stream, p_request->i_segment, &download_info) )
    {
        print_dbg(__func__,
                  "[HLSContent] downloadHLSPiece - We don't have the info to download the piece %d-%d (from segment %d) has expired",
                  p_request->i_stream,
                  p_request->i_piece,
                  p_request->i_segment);
        //goto LABEL_4;
        this->p_parent->notifyPieceReady(p_request, 0);
        return 0;
    }
    if ( download_info.b_is_downloaded )
    {
        print_dbg(__func__,
                  "[HLSContent] downloadHLSPiece - The segment %d-%d has been already downloaded",
                  p_request->i_stream,
                  p_request->i_segment);
        free(download_info.psz_url);
        this->p_parent->notifyPieceReady(p_request, 0);
        result = 1;
    }
    else
    {
        print_msg(__func__,
                  "[HLSContent] downloadHLSPiece - Downloading piece %d-%d (from segment %d)",
                  p_request->i_stream,
                  p_request->i_piece,
                  p_request->i_segment);
        //_FST7 = 0.5 * download_info.f_duration;
        v23 = (v22 & 0xF3FF) | 0x800;
        //__asm { frndint }
        v7 = mdate();
        p_data = sendHTTPRangeRequest(
                this->p_this,
                download_info.psz_url,
                p_request->i_offset,
                p_request->i_size,
                0.5 * download_info.f_duration);
        v8 = mdate();
        if ( !p_data )
        {
            print_dbg(__func__,
                      "[HLSContent] downloadHLSPiece - ERROR: Can't download piece %d-%d (from segment %d)",
                      p_request->i_stream,
                      p_request->i_piece,
                      p_request->i_segment
            );
            free(download_info.psz_url);
            //goto LABEL_4;
            this->p_parent->notifyPieceReady(p_request, 0);
            return 0;
        }
        if ( p_data->i_buffer_index != p_request->i_size )
        {
            print_err(__func__,
                      "[HLSContent] downloadHLSPiece - it seems that the server doesn't supports HTTP Ranges (we requested %lld bytes and we got %u)",
                      p_request->i_size,
                      p_data->i_buffer_index);
            this->p_this->p_structure->b_hls_can_down_pieces = false;
            block_Release(p_data);
            free(download_info.psz_url);
            this->p_parent->notifyPieceReady(p_request, 0);
            result = 0;
        }
        else
        {
            v10 = v8 - v7;
            print_dbg(__func__,
                      "[HLSContent] downloadHLSPiece - The piece %d-%d (from segment %d) was downloaded in %.02f secs",
                      p_request->i_stream,
                      p_request->i_piece,
                      p_request->i_segment,
                      v10 / 1000000.0);
            downloaded_info.p_data = p_data;
            downloaded_info.t_downloaded = v10;
            this->p_parent->notifyPieceReady(p_request, &downloaded_info);
            free(download_info.psz_url);
            result = 1;
        }
    }
    return result;
}

void HLSContent::setSegmentData(const int i_current_stream, const int i_chosen_sequence,
                                hls_segment_t *p_new_segment) {
    HLSManager *v4; // eax@1
    const hls_stream_t *v5; // eax@3
    hls_segment_t *v6; // edi@4
    //int v8; // edx@5
    //int v10; // edx@5
    //int v12; // edx@5
    long long v13; // eax@5
    //int v14; // edx@5
    long long v15; // eax@5
    HLSManager *v16; // eax@6

    v4 = this->p_parent;
    if ( v4 )
        Mypthread_mutex_lock(81, &v4->content_lock);
    v5 = getStream(this->p_content, i_current_stream);
    if ( v5 )
    {
        v6 = findSegment(v5, i_chosen_sequence);
        if ( v6 )
        {
            //v8 = HIDWORD(p_new_segment->i_size);
            v6->p_data = block_Duplicate(p_new_segment->p_data);
            //HIDWORD(v6->i_size) = v8;
            //v10 = HIDWORD(p_new_segment->i_bitrate);
            v6->i_size = p_new_segment->i_size;//LODWORD(v6->i_size) = v9;
            //HIDWORD(v6->i_bitrate) = v10;
            //v12 = HIDWORD(p_new_segment->t_downloaded);
            v6->i_bitrate = p_new_segment->i_bitrate;//LODWORD(v6->i_bitrate) = v11;
            v6->f_duration = p_new_segment->f_duration;
            //HIDWORD(v6->t_downloaded) = v12;
            //v14 = HIDWORD(p_new_segment->t_download);
            v6->t_downloaded = p_new_segment->t_downloaded;////LODWORD(v6->t_downloaded) = v13;
            //HIDWORD(v6->t_download) = v14;
            v6->t_download = p_new_segment->t_download;//LODWORD(v6->t_download) = v15;
        }
    }
    v16 = this->p_parent;
    if ( v16 )
        Mypthread_mutex_unlock(81, &v16->content_lock);
}

bool HLSContent::getSegmentDownloadInfo(const int i_current_stream, const int i_chosen_sequence,
                                        segment_download_info_t_0 *p_download_info) {
    HLSManager *v4; // eax@1
    hls_stream_t *v5; // ebp@3
    hls_segment_t *v7; // edi@4
    char *v8; // eax@5
    //int v9; // edx@5
    float v10; // eax@5
    block_t *v11; // edi@5
    //int v12; // eax@5
    HLSManager *v13; // eax@6
    bool b_result; // [sp+1Fh] [bp-1Dh]@3

    v4 = this->p_parent;
    if ( v4 )
        Mypthread_mutex_lock(82, &v4->content_lock);
    b_result = 0;
    v5 = getStream(this->p_content, i_current_stream);
    if ( v5 )
    {
        v7 = findSegment(v5, i_chosen_sequence);
        if ( v7 )
        {
            v8 = (char *)strdup(v7->psz_url);
            //v9 = HIDWORD(v5->i_bitrate);
            b_result = 1;
            p_download_info->psz_url = v8;
            v10 = v7->f_duration;
            v11 = v7->p_data;
            p_download_info->f_duration = v10;
            //v12 = v5->i_bitrate;
            //HIDWORD(p_download_info->i_bitrate) = v9;
            p_download_info->b_is_downloaded = v11 != 0;
            //LODWORD(p_download_info->i_bitrate) = v12;
            p_download_info->i_bitrate = v5->i_bitrate;
        }
    }
    v13 = this->p_parent;
    if ( v13 )
        Mypthread_mutex_unlock(82, &v13->content_lock);
    return b_result;
}

void Destroy_Queue(generic_queue_t_0 *queue)
{
    queue_node_t *v1; // eax@1
    queue_node_t *v2; // esi@4

    pthread_mutex_destroy(&queue->lock);
    v1 = queue->p_first;
    if ( v1 )
    {
        while ( 1 )
        {
            v2 = v1->p_next;
            free(v1);
            if ( !v2 )
                break;
            v1 = v2;
        }
    }
    free(queue);
}

HLSContent::~HLSContent() {
    if ( this->p_events_queue )
    {
        Destroy_Queue(this->p_events_queue);
        this->p_events_queue = 0;
    }
    if ( this->p_content )
        freeContent(this->p_content);
    if ( this->p_parser )
    {
        delete this->p_parser;
    }
}

char * ReadLine(char *buffer, char **pos, const size_t len)
{
    char **v3; // ebx@1
    char *v4; // edi@2
    int v5; // edx@2
    char *v6; // esi@2
    char *result; // eax@10

    v3 = pos;
    if ( buffer )
    {
        v4 = &buffer[len];
        v5 = 0;
        v6 = buffer;
        if ( buffer < &buffer[len] )
        {
            if ( *buffer != 0xA && *buffer )
            {
                do
                    ++v6;
                while ( v6 != v4 && *v6 && *v6 != 0xA );
                v5 = v6 - buffer;
            }
            else
            {
                v6 = buffer;
                v5 = 0;
            }
        }
        result = strndup(buffer, v5);
        if ( *v6 )
            *v3 = v6 + 1;
        else
            *v3 = v4;
    }
    else
    {
        result = 0;
    }
    return result;
}

hls_content_t* newContent(const char *psz_url)
{
    hls_content_t *v1; // ebx@1
    char *v2; // eax@4

    v1 = 0;
    if ( psz_url )
    {
        if ( *psz_url )
        {
            v1 = new hls_content_t();
            if ( v1 )
            {
                v2 = strdup(psz_url);
                v1->b_has_metafile = false;
                v1->b_is_live = true;
                v1->i_version = 1;
                v1->psz_main_url = v2;
                v1->p_streams = array_new<hls_stream_t>();
            }
        }
    }
    return v1;
}

hls_content_t* HLSParser::parseManifest(char *p_buffer, size_t i_length, const char *psz_url)
{
    char *v5; // ebp@4
    char *v6; // eax@4
    //const char *v8; // edi@5
    //signed int v9; // ecx@5
    //char *v10; // esi@5
    hls_content_t *v11; // esi@9
    //bool v14; // di@10
    char *v15; // eax@10
    char *v17; // edi@11
    hls_stream_t *v18; // eax@17
    int version; // [sp+38h] [bp-24h]@10
    uint8_t *tmp; // [sp+3Ch] [bp-20h]@11

    if ( !p_buffer )
        return 0;
    if ( !i_length )
        return 0;
    if ( !*psz_url )
        return 0;
    v5 = &p_buffer[i_length];
    v6 = ReadLine(p_buffer, &p_buffer, i_length);
    //    v7 = v6 == 0;
    if ( !v6 )
        return 0;
    if ( strncmp(v6, "#EXTM3U", 7) )
    {
        print_err(__func__, "[HLSParser] missing #EXTM3U tag .. aborting");
        free(v6);
        return 0;
    }
    free(v6);
    v11 = newContent(psz_url);
    if ( !v11 )
        return v11;
    version = 1;
    if (strstr(p_buffer, "#EXT-X-STREAM-INF")) {
        v11->b_has_metafile = true;
    } else {
        v11->b_has_metafile = false;
    }
    //v14 = v13 != 0;
    v15 = strstr(p_buffer, "#EXT-X-VERSION:");
    v11->i_version = 1;
    if ( v15 )
    {
        tmp = 0;
        v17 = ReadLine(v15, (char **)&tmp, v5 - v15);
        if ( v17 )
        {
            if ( sscanf(v17, "#EXT-X-VERSION:%d", &version) != 1 )
            {
                print_wrn(__func__, "#EXT-X-VERSION: no protocol version found, assuming version 1.");
                version = 1;
            }
            free(v17);
        }
        v11->i_version = version;
        //v14 = v11->b_has_metafile;
    }
    if ( v11->b_has_metafile ) {
        /**
         * new stream array
         * "#EXT-X-STREAM-INF"
         */
        array_destroy(v11->p_streams);
        v11->p_streams = parseMetaPlaylist(p_buffer, i_length, version, psz_url);
    } else {
        /**
         * add one stream with segment array in stream array
         * "#EXT-X-TARGETDURATION:"
         */
        v18 = parsePlaylist(p_buffer, i_length, v11->i_version, psz_url);
        if ( v18 ) {
            array_append(v11->p_streams, v18);
        }
    }
    return v11;
}

HLSParser::HLSParser(goalbit_t_0 *p_goalbit) {
    this->p_this = p_goalbit;
    return;
}

hls_stream_t* newStream(const int id, const uint64_t bw, const char *uri)
{
    hls_stream_t *v3; // ebx@1
    char *v4; // eax@4

    v3 = 0;
    if ( uri )
    {
        if ( *uri )
        {
            v3 = new hls_stream_t();// *)malloc(0x4C);
            if ( v3 )
            {
                v3->i_bitrate = bw;
                v3->f_target_duration = 0;
                v3->i_program_id = id;
                v3->i_total_size = 0;
                v3->i_media_sequence = 0;
                v3->i_version = 1;
                v3->b_cache = 1;
                v3->b_end_list = 0;
                v4 = strdup(uri);
                v3->psz_codecs = 0;
                v3->psz_resolution = 0;
                v3->psz_manifest_url = v4;
                v3->p_segments = array_new<hls_segment_t>();
                pthread_mutex_init(&v3->lock, 0);
            }
        }
    }
    return v3;
}

hls_segment_t* newSegment(const int id, const float duration, const uint64_t size, const uint64_t bw, const char *url, const char *desc, const bool discontinuity, const char *date_time)
{
    hls_segment_t *v8; // ebx@1
    char *v9; // eax@4
    char *v10; // eax@6

    v8 = 0;
    if ( url )
    {
        if ( *url )
        {
            v8 = new hls_segment_t();
            if ( v8 )
            {
                v8->t_downloaded = 0;
                v8->i_sequence = id;
                v8->i_size = size;
                v8->t_download = 0;
                v8->f_duration = duration;
                v8->i_bitrate = bw;
                v8->b_discontinuity = discontinuity;
                v8->i_size = size;
                //v8->i_bitrate = bw;
                v8->psz_url = (char *)strdup(url);
                v9 = 0;
                if ( desc ) {
                    v9 = (char *) strdup(desc);
                }
                v8->psz_desc = v9;
                v10 = 0;
                if ( date_time )
                    v10 = (char *)strdup(date_time);
                v8->psz_date = v10;
                v8->p_data = 0;
                pthread_mutex_init(&v8->lock, 0);
            }
        }
    }
    return v8;
}

bool addSegmentToStream(hls_stream_t *p_hls, const float duration, const char *uri, const char *desc, const bool discontinuity, const char *date_time)
{
    const char *v6; // ebp@1
    int v7; // eax@3
    char *v8; // edx@3
    bool v9; // zf@3
    const char *v10; // edi@4
    signed int v11; // ecx@4
    const char *v12; // esi@4
    char *v13; // eax@8
    int v14; // edi@9
    int v15; // ST3C_4@9
    int v16; // eax@9
    char *v17; // esi@9
    bool v18; // al@9
    hls_segment_t *v19; // eax@11
    hls_segment_t *v20; // edi@11
    const char *src; // [sp+38h] [bp-34h]@8
    int v23; // [sp+48h] [bp-24h]@3
    int v24; // [sp+4Ch] [bp-20h]@3

    v6 = uri;
    if ( !p_hls )
        _assert_fail(
                "p_hls",
                "src/basetools/stream.cpp",
                0x1BB,
                "bool addSegmentToStream(hls_stream_t*, float, const char*, const char*, bool, const char*)");
    if ( !uri )
        _assert_fail(
                "uri",
                "src/basetools/stream.cpp",
                0x1BC,
                "bool addSegmentToStream(hls_stream_t*, float, const char*, const char*, bool, const char*)");
    Mypthread_mutex_lock(83, &p_hls->lock);
    v23 = p_hls->i_media_sequence;
    v7 = array_count(p_hls->p_segments);
    v8 = p_hls->psz_manifest_url;
    v9 = v8 == 0;
    v24 = v7;
    if ( !v8 )
        _assert_fail(
                "psz_url != __null && psz_path != __null",
                "src/basetools/../basetools/strings.h",
                0x71,
                "char* relative_URI(const char*, const char*)");
    v10 = "http";
    v11 = 4;
    v12 = uri;
    do
    {
        if ( !v11 )
            break;
        v9 = *v12++ == *v10++;
        --v11;
    }
    while ( v9 );
    if ( v9 || (src = p_hls->psz_manifest_url, (v13 = strrchr(v8, '/')) == 0) )
    {
        v18 = discontinuity;
        v17 = 0;
    }
    else
    {
        v14 = v13 - src;
        v15 = v13 - src + 1;
        v16 = strlen(uri);
        v17 = (char *)malloc(v14 + v16 + 2);
        strncpy(v17, src, v15);
        v17[v14 + 1] = 0;
        strcat(v17, uri);
        v18 = discontinuity;
        if ( v17 )
            v6 = v17;
    }
    v19 = newSegment(v24 + v23, duration, 0LL, 0LL, v6, desc, v18, date_time);
    v20 = v19;
    if ( v19 )
        array_append(p_hls->p_segments, v19);
    free(v17);
    Mypthread_mutex_unlock(83, &p_hls->lock);
    return v20 != 0;
}

/**
 * new stream with segment array
 * @param p_buffer
 * @param i_length
 * @param i_version
 * @param psz_uri
 * @return
 */
hls_stream_t* HLSParser::parsePlaylist(char *p_buffer, size_t i_length, int i_version, const char *psz_uri) {
    size_t v5; // ebx@1
    char *v6; // ebp@7
    char *v7; // eax@7
    char *v8; // eax@8
    char *v9; // ebx@8
    char *v10; // eax@11
    bool v11; // zf@11
    char *v12; // ebx@11
    char *v15; // esi@12
    char v34; // al@40
    char v35; // si@40
    bool v36; // al@42
    char *p_end; // [sp+20h] [bp-3Ch]@7
    hls_stream_t *p_hls; // [sp+24h] [bp-38h]@4
    char *psz_date; // [sp+28h] [bp-34h]@10
    bool b_discontinuity; // [sp+2Fh] [bp-2Dh]@10
    uint8_t *p_rest; // [sp+34h] [bp-28h]@8
    float segment_duration; // [sp+38h] [bp-24h]@10
    char *psz_desc; // [sp+3Ch] [bp-20h]@10

    v5 = i_length;
    if ( p_buffer && i_length && *p_buffer )
    {
        p_hls = 0;
        if ( !psz_uri )
            return p_hls;
        if ( !*psz_uri )
            return p_hls;
        p_hls = newStream(0, 0LL, psz_uri);
        if ( !p_hls )
            return p_hls;
        v6 = p_buffer;
        p_hls->i_version = i_version;
        p_end = &v6[v5];
        v7 = strstr(v6, "#EXT-X-TARGETDURATION:");
        if ( !v7 )
        {
            LABEL_10:
            segment_duration = 0.0;
            psz_desc = 0;
            psz_date = 0;
            b_discontinuity = 0;
            do
            {
                v10 = ReadLine(v6, &p_buffer, p_end - v6);
                v11 = v10 == 0;
                v12 = v10;
                if ( !v10 )
                    break;
                v6 = p_buffer;
                v15 = v10;
                v11 = strncmp(v15, "#EXT-X-MEDIA-SEQUENCE", sizeof("#EXT-X-MEDIA-SEQUENCE")-1);
                if ( !v11 )
                {
                    v35 = 1;
                    p_hls->i_media_sequence = parseMediaSequence(v10);
                }
                else
                {
                    v11 = strncmp(v10, "#EXT-X-PROGRAM-DATE-TIME", sizeof("#EXT-X-PROGRAM-DATE-TIME")-1);
                    if ( !v11 )
                    {
                        v35 = 1;
                        psz_date = parseProgramDateTime(v10);
                    }
                    else
                    {
                        v11 = strncmp(v10, "#EXT-X-ALLOW-CACHE", sizeof("#EXT-X-ALLOW-CACHE")-1);
                        if ( !v11 )
                        {
                            v35 = 1;
                            p_hls->b_cache = parseAllowCache(v10);
                        }
                        else
                        {
                            v11 = strncmp(v10, "#EXT-X-DISCONTINUITY", sizeof("#EXT-X-DISCONTINUITY")-1);
                            if ( !v11 )
                            {
                                v35 = 1;
                                parseDiscontinuity(v10);
                                b_discontinuity = 1;
                            }
                            else
                            {
                                v11 = strncmp(v10, "#EXT-X-VERSION", sizeof("#EXT-X-VERSION")-1);
                                if ( !v11 )
                                {
                                    v35 = 1;
                                    p_hls->i_version = parseVersion(v10);
                                }
                                else
                                {
                                    v11 = strncmp(v10, "#EXT-X-ENDLIST", sizeof("#EXT-X-ENDLIST")-1);
                                    if ( !v11 )
                                    {
                                        v35 = 1;
                                        p_hls->b_end_list = 1;
                                    }
                                    else
                                    {
                                        v11 = strncmp(v10, "#EXTINF", sizeof("#EXTINF")-1);
                                        if ( !v11 )
                                        {
                                            v35 = parseSegmentInformation(
                                                    v10,
                                                    p_hls->i_version,
                                                    &segment_duration,
                                                    &psz_desc);
                                        }
                                        else
                                        {
                                            v34 = *v10;
                                            v35 = 1;
                                            if ( v34 != '#' && v34 )
                                            {
                                                v36 = addSegmentToStream(p_hls, segment_duration, v12, psz_desc, b_discontinuity, psz_date);
                                                segment_duration = 0.0;
                                                v35 = v36;
                                                if ( psz_desc )
                                                {
                                                    free(psz_desc);
                                                    psz_desc = 0;
                                                }
                                                if ( psz_date )
                                                {
                                                    free(psz_date);
                                                    psz_date = 0;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                free(v12);
                if ( p_end <= v6 )
                    break;
            } while ( v35 );
            if ( psz_desc )
                free(psz_desc);
            return p_hls;
        }
        p_rest = 0;
        v8 = ReadLine(v7, (char **)&p_rest, p_end - v7);
        v9 = v8;
        if ( v8 )
        {
            p_hls->f_target_duration = parseTargetDuration(v8);
            free(v9);
            goto LABEL_10;
        }
    }
    return 0;
}

int HLSParser::parseMediaSequence(char *p_read) {
    int result; // eax@4
    int sequence; // [sp+1Ch] [bp-10h]@2

    if ( p_read )
    {
        sequence = -1;
        if ( sscanf(p_read, "#EXT-X-MEDIA-SEQUENCE:%d", &sequence) != 1 )
            print_err(__func__, "expected #EXT-X-MEDIA-SEQUENCE:<s>");
        result = sequence;
    }
    else
    {
        result = -1;
    }
    return result;
}

char *HLSParser::parseProgramDateTime(char *p_read) {
    char *v2; // ebx@1
    const char *v3; // eax@1
    const char *v4; // esi@1
    int v5; // eax@2
    char *p_next; // [sp+1Ch] [bp-10h]@1

    print_dbg(__func__, "tag not supported: #EXT-X-PROGRAM-DATE-TIME %s", p_read);
    v2 = 0;
    p_next = 0;
    v3 = (const char *)strtok_r(p_read, ":", &p_next);
    v4 = v3;
    if ( v3 )
    {
        v5 = strlen(v3);
        v2 = (char *)malloc(v5 + 1);
        strcpy(v2, v4);
    }
    return v2;
}

int HLSParser::parseAllowCache(char *p_read) {
    int v5; // ebx@1
    char *v6; // esi@2
    int v7; // eax@2
    bool v8; // zf@2
    const char *v9; // edi@3
    signed int v10; // ecx@3
    char answer[4]; // [sp+18h] [bp-14h]@2
    //int v13; // [sp+1Ch] [bp-10h]@1
    //int v14; // [sp+20h] [bp-Ch]@1
    //int v15; // [sp+24h] [bp-8h]@1
    //int v16; // [sp+28h] [bp-4h]@1

    //v14 = a1;
    v5 = 0;
    //v13 = *MK_FP(__GS__, 0x14);
    //v16 = a2;
    //v15 = a3;
    if ( p_read )
    {
        v6 = answer;
        answer[0] = 0;
        v7 = sscanf(p_read, "#EXT-X-ALLOW-CACHE:%3s", answer);
        v8 = v7 == 1;
        if ( v7 == 1 )
        {
            v9 = "NO";
            v10 = 2;
            do
            {
                if ( !v10 )
                    break;
                v8 = *v6++ == *v9++;
                --v10;
            }
            while ( v8 );
            v5 = !v8;
        }
        else
        {
            print_err(__func__, "#EXT-X-ALLOW-CACHE, ignoring ...");
        }
    }
    return v5;
}

int HLSParser::parseDiscontinuity(char *p_read) {
    if ( p_read )
        print_dbg(__func__, "#EXT-X-DISCONTINUITY %s", p_read);
    return 0;
}

int HLSParser::parseVersion(char *p_read) {
    int result; // eax@1
    int version; // [sp+1Ch] [bp-10h]@2

    result = 1;
    if ( p_read )
    {
        version = 1;
        if ( sscanf(p_read, "#EXT-X-VERSION:%d", &version) != 1 )
            print_err(__func__, "#EXT-X-VERSION: no protocol version found, should be version 1.");
        result = version;
        if ( (unsigned int)(version - 1) > 2 )
        {
            print_err(__func__, "#EXT-X-VERSION should be version 1, 2 or 3 iso %d", version);
            result = version;
        }
    }
    return result;
}

int HLSParser::parseSegmentInformation(char *p_read, int version, float *duration, char **desc) {
    signed int v5; // ebx@1
    char* v6; // eax@3
    const char *v7; // esi@7
    int v8; // eax@10
    char *v9; // eax@10
    int value; // [sp+14h] [bp-38h]@12
    char *p_next; // [sp+28h] [bp-24h]@2
    float d; // [sp+2Ch] [bp-20h]@5

    v5 = 0;
    if ( !p_read )
        return v5;
    p_next = 0;
    if ( !strtok_r(p_read, ":", &p_next) )
        return v5;
    v6 = strtok_r(0, ",", &p_next);
    if ( !v6 )
        return v5;
    if ( version > 2 )
    {
        d = 0.0;
        if ( sscanf(v6, "%f", &d) != 0xFFFFFFFF )
        {
            *duration = d;
            goto LABEL_7;
        }
        LABEL_14:
        *(_DWORD *)duration = 0;
        return v5;
    }
    value = strtol(v6, 0, 0xA);
    if ( errno == 0x22 )
        goto LABEL_14;
    *duration = (long double)value;
    LABEL_7:
    v5 = 1;
    v7 = (const char *)strtok_r(0, ",", &p_next);
    if ( v7 )
    {
        if ( *desc )
            free(*desc);
        v5 = 1;
        v8 = strlen(v7);
        v9 = (char *)malloc(v8 + 1);
        *desc = v9;
        strcpy(v9, v7);
    }
    return v5;
}

long double HLSParser::parseTargetDuration(char *p_read) {
    long double result; // fst7@1
    float duration; // [sp+1Ch] [bp-10h]@2

    result = -1.0;
    if ( p_read )
    {
        duration = 0.0;
        if ( sscanf(p_read, "#EXT-X-TARGETDURATION:%f", &duration) != 1 )
            print_err(__func__, "expected #EXT-X-TARGETDURATION:<s>");
        result = duration;
    }
    return result;
}

array_t<hls_stream_t>* HLSParser::parseMetaPlaylist(char *p_buffer, size_t i_length, int i_version, const char *psz_url) {
    char *v5; // ebx@1
    char *v6; // eax@6
    bool v7; // zf@6
    char *v12; // eax@11
    char *v13; // ebx@11
    hls_stream_t *v14; // eax@13
    array_t<hls_stream_t> *result; // eax@17
    char *p_end; // [sp+18h] [bp-24h]@4
    array_t<hls_stream_t> *p_streams; // [sp+1Ch] [bp-20h]@4

    v5 = p_buffer;
    if ( p_buffer && i_length && *p_buffer )
    {
        p_end = &p_buffer[i_length];
        p_streams = array_new<hls_stream_t>();
        while ( 1 )
        {
            v6 = ReadLine(v5, &p_buffer, p_end - v5);
            if ( !v6 ) {
                return 0;
            }
            v5 = p_buffer;
            v7 = strncmp(v6, "#EXT-X-STREAM-INF", sizeof("#EXT-X-STREAM-INF")-1);
            if ( !v7 )
            {
                v12 = ReadLine(p_buffer, &p_buffer, p_end - p_buffer);
                v13 = v12;
                if ( v12 )
                {
                    if ( *v12 == '#' )
                    {
                        print_msg(__func__, "[HLSParser] Skipping invalid stream-inf: %s", v12);
                        free(v13);
                    }
                    else
                    {
                        v14 = parseStreamInformation(v6, psz_url, v12);
                        if ( v14 )
                        {
                            v14->i_version = i_version;
                            array_append(p_streams, v14);
                        }
                        free(v13);
                    }
                }
                v5 = p_buffer;
                free(v6);
                if ( v5 >= p_end )
                    break;
            }
            else
            {
                free(v6);
                if ( v5 >= p_end )
                    break;
            }
        }
        result = p_streams;
    }
    else
    {
        result = 0;
    }
    return result;
}

char* parse_Attribute(const char *line, const char *attr)
{
    const char *v2; // esi@1
    const char *v3; // ebx@1
    const char *v4; // ebp@1
    char *v5; // ebx@1
    size_t v6; // edi@2
    char *result; // eax@5
    char *v8; // eax@7
    char *v9; // edx@7
    char *v10; // ecx@7

    v2 = attr;
    v3 = line;
    v4 = &line[strlen(line)];
    v5 = (char*)strchr(v3, ':');
    if ( v5 )
    {
        v6 = strlen(v2);
        while ( strncasecmp(v5, v2, v6) )
        {
            if ( v4 <= ++v5 ) {
                //goto LABEL_5;
                return 0;
            }
        }
        v8 = strchr(v5, ',');
        v9 = &v5[v6 + 1];
        v10 = v8;
        result = 0;
        if ( v4 > v9 )
        {
            if ( v10 )
                result = strndup(&v5[v6 + 1], v10-v9);
            else
                result = strndup(&v5[v6 + 1], v4-v9);
        }
    }
    else
    {
        //LABEL_5:
        //result = 0;
        return 0;
    }
    return result;
}

hls_stream_t *HLSParser::parseStreamInformation(char *p_read, const char *url, const char *uri)
{
    char *v4; // eax@3
    hls_stream_t *v5; // esi@3
    char *v6; // eax@4
    //uint64_t v7; // rax@5
    char *v8; // ebp@6
    bool v9; // cf@7
    bool v10; // zf@7
    const char *v11; // esi@8
    const char *v12; // edi@8
    signed int v13; // ecx@8
    const char *v14; // esi@11
    char *v15; // edi@11
    char *v16; // eax@12
    int v17; // eax@13
    int v18; // ST1C_4@13
    int v19; // ST20_4@13
    int v20; // eax@13
    char *v21; // eax@15
    char *v22; // eax@17
    hls_stream_t *result; // eax@19
    uint64_t bw; // [sp+24h] [bp-28h]@5
    int id; // [sp+2Ch] [bp-20h]@4

    if ( p_read && *p_read )
    {
        v4 = parse_Attribute(p_read, "PROGRAM-ID");
        //v5 = (hls_stream_t *)v4;
        if ( v4 )
        {
            id = strtol(v4, 0, '\n');
            free(v4);
            v6 = parse_Attribute(p_read, "BANDWIDTH");
            //v5 = (hls_stream_t *)v6;
            if ( v6 )
            {
                bw = strtol(v6, 0, '\n');
                //bw = v7;
                free(v6);
                if ( bw )
                {
                    v8 = strdup(url);
                    if ( !v8 || (v9 = 0, v10 = uri == 0, !uri) )
                        _assert_fail(
                                "psz_url != __null && psz_path != __null",
                                "src/basetools/../basetools/strings.h",
                                0x71,
                                "char* relative_URI(const char*, const char*)");
                    v11 = uri;
                    v12 = "http";
                    v13 = 4;
                    do
                    {
                        if ( !v13 )
                            break;
                        v9 = (const unsigned __int8)*v11 < *v12;
                        v10 = *v11++ == *v12++;
                        --v13;
                    } while ( v10 );
                    v14 = uri;
                    v15 = 0;
                    if ( (!v9 && !v10) != v9 )
                    {
                        v16 = strrchr(v8, '/');
                        if ( v16 )
                        {
                            v17 = v16 - v8;
                            v18 = v17 + 1;
                            v19 = v17;
                            v20 = strlen(uri);
                            v15 = (char *)malloc(v19 + v20 + 2);
                            strncpy(v15, v8, v18);
                            v15[v19 + 1] = 0;
                            strcat(v15, uri);
                            if ( v15 )
                                v14 = v15;
                        }
                    }
                    v5 = newStream(id, bw, v14);
                    free(v8);
                    free(v15);
                    v21 = parse_Attribute(p_read, "CODECS");
                    if ( v21 )
                        v5->psz_codecs = v21;
                    v22 = parse_Attribute(p_read, "RESOLUTION");
                    if ( v22 )
                        v5->psz_resolution = v22;
                }
                else
                {
                    v5 = 0;
                    print_err(__func__, "#EXT-X-STREAM-INF: bandwidth cannot be 0");
                }
            }
            else
            {
                print_err(__func__, "#EXT-X-STREAM-INF: expected BANDWIDTH=<value>");
            }
        }
        else
        {
            print_err(__func__, "#EXT-X-STREAM-INF: expected PROGRAM-ID=<value>");
        }
        result = v5;
    }
    else
    {
        result = 0;
    }
    return result;
}

BWRate::BWRate()
{
    int v1; // eax@1

    v1 = 0;
    this->i_total_bytes = 0LL;
    this->i_total_requests = 0;
    do
    {
        this->history[v1].bytes = 0;
        this->history[v1].timestamp = 0;
        ++v1;
    } while ( v1 != sizeof(history)/ sizeof(bandwidth) );
    this->i_history_last = 0;
    return;
}

void BWRate::reset() {
    int v1; // eax@1

    v1 = 0;
    this->i_total_bytes = 0LL;
    this->i_total_requests = 0;
    do
    {
        this->history[v1].bytes = 0;
        this->history[v1].timestamp = 0;
        ++v1;
    }
    while ( v1 != 4 );
    this->i_history_last = 0;

}

uint64_t BWRate::getTotalBytes() {
    return this->i_total_bytes;
}

void BWRate::addBytes(size_t i_bytes, mtime_t t_time) {
    int v3; // edx@1
    bandwidth *v4; // esi@1

    v3 = this->i_history_last;
    this->i_total_bytes += i_bytes;
    v4 = &this->history[v3];
    ++this->i_total_requests;
    v4->bytes = i_bytes;
    v4->timestamp = t_time;
    this->i_history_last++;
    if (this->i_history_last > 3) {
        this->i_history_last = 0;
    }
}

btCache::btCache(goalbit_t_0 *p_goalbit) {
    //goalbit_t_0 *v3; // esi@1
    //goalbit_structure_t *v4; // eax@1
    size_t v5; // edi@1
    CachePiece **v7; // eax@1
    size_t v8; // ecx@2
    size_t v9; // edx@2

    this->p_this = p_goalbit;
    //v2 = (btFiles *)operator new(0xCu);
    //btFiles::btFiles(v2, this->p_this);
    this->p_files = new btFiles(this->p_this);
    //v3 = this->p_this;
    //v4 = this->p_this->p_structure;
    v5 = p_this->p_structure->i_p2p_win_length;
    this->i_piece_offset = p_this->p_structure->i_p2p_win_offset;
    this->i_cache_used = 0;
    this->p_oldest = 0;
    this->p_newest = 0;
    this->i_cache_size = p_this->p_param->i_cache_size;
    this->p_toflush = 0;
    this->i_unflushed = 0;
    this->b_flush_failed = 0;
    v7 = (CachePiece **)malloc(sizeof(void*) * v5);
    this->p_pieces = v7;
    if ( v7 )
    {
        v8 = 0;
        v9 = 0;
        if ( v5 )
        {
            while ( 1 )
            {
                v7[v8] = 0;
                v8 = ++v9;
                if ( v9 >= this->p_this->p_structure->i_p2p_win_length )
                    break;
                v7 = this->p_pieces;
            }
        }
    }
    else
    {
        print_err(__func__, "Allocate cache memory failed");
    }
}

void btCache::FlushAll()
{
    bool v1; // di@1
    //goalbit_t_0 *v2; // ecx@1
    size_t v3; // eax@2
    size_t v4; // ebx@2
    CachePiece *v5; // eax@3

    v1 = 0;
    print_dbg(__func__, "Flushing all pieces from cache");
    //v2 = this->p_this;
    if ( this->p_this->p_structure->i_p2p_win_length )
    {
        v3 = 0;
        v4 = 0;
        do
        {
            v5 = this->p_pieces[v3];
            if ( v5 )
            {
                FlushPiece(v5);
                if ( !v1 && this->b_flush_failed )
                    v1 = 1;
                //v2 = this->p_this;
            }
            v3 = ++v4;
        }
        while ( v4 < p_this->p_structure->i_p2p_win_length );
    }
    this->b_flush_failed = v1;
    if ( this->p_toflush )
        free(this->p_toflush);
    this->p_toflush = 0;
}

void btCache::FlushPiece(CachePiece *p_piece)
{
    size_t v2; // eax@2
    //goalbit_structure_t *v3; // ebp@2
    size_t v4; // ecx@2
    unsigned int v5; // edx@2
    unsigned int v6; // ecx@6
    bool v7; // bp@6
    block_t *v8; // eax@10
    CachePiece *v9; // eax@10
    bool v10; // [sp+2Bh] [bp-21h]@2

    if ( p_piece )
    {
        v2 = p_piece->i_piece_id;
        //v3 = this->p_this->p_structure;
        v4 = p_this->p_structure->i_p2p_win_offset;
        if ( v2 >= v4 )
        {
            if ( v4 == 0x80000001 )
                goto LABEL_22;
            v6 = (p_this->p_structure->i_p2p_win_length + v4);
            if ( v2 <= v6 )
            {
                LABEL_22:
                print_dbg(__func__,
                          "Flushing piece %d, quality %d (%d/%d)",
                          v2,
                          p_piece->i_piece_quality,
                          p_piece->p_block->i_buffer_index,
                          p_piece->p_block->i_buffer_size);
                if ( this->p_files->writePiece(p_piece->i_piece_quality, p_piece->i_piece_id, p_piece->p_block) )
                {
                    this->b_flush_failed = 1;
                }
                else
                {
                    v8 = p_piece->p_block;
                    this->b_flush_failed = 0;
                    this->i_unflushed -= v8->i_buffer_size;
                    v9 = this->p_toflush;
                    p_piece->b_flushed = 1;
                    if ( v9
                         && p_piece->i_piece_id == v9->i_piece_id
                         && p_piece->i_piece_quality == v9->i_piece_quality
                         && v9->b_flushed )
                    {
                        while ( 1 )
                        {
                            v9 = v9->p_newer;
                            if ( !v9 )
                                break;
                            if ( !v9->b_flushed )
                            {
                                this->p_toflush = v9;
                                return;
                            }
                        }
                        this->p_toflush = 0;
                    }
                }
            }
        }
    }
}

void btCache::UpdateWindow() {
    //goalbit_structure_t *v1; // eax@1
    //size_t v2; // edx@2
    size_t v4; // ebx@5
    CachePiece *v5; // edx@7
    size_t v6; // edi@8
    //unsigned int v7; // eax@8
    //bool v8; // bp@8
    //size_t v9; // edx@12
    size_t v10; // [sp+14h] [bp-28h]@8
    //goalbit_t_0 *v11; // [sp+18h] [bp-24h]@1

    this->p_files->window_changed();
    //v11 = this->p_this;
    //v1 = this->p_this->p_structure;
    if ( p_this->p_structure->b_p2p_enabled )
    {
        //v2 = this->i_piece_offset;
        if ( i_piece_offset == 0x80000001 )
        {
            //v9 = p_this->p_structure->i_p2p_win_offset;
            if ( p_this->p_structure->i_p2p_win_offset != 0x80000001 )
            {
                this->i_piece_offset = p_this->p_structure->i_p2p_win_offset;
                return;
            }
        }
        else if ( i_piece_offset == p_this->p_structure->i_p2p_win_offset )
        {
            return;
        }
        if ( p_this->p_structure->i_p2p_win_length )
        {
            v4 = 0;
            do
            {
                v5 = this->p_pieces[v4];
                if ( v5 )
                {
                    v6 = v5->i_piece_id;
                    v10 = p_this->p_structure->i_p2p_win_offset;
                    //                    v7 = (((signed int)(v6 - v10) >> 0x1F) ^ (v6 - v10)) - ((signed int)(v6 - v10) >> 0x1F);
                    //                    v8 = v5->i_piece_id > v10;
                    //                    if ( v7 < 0x80000000 - v7 )
                    //                        v8 = v6 < v10;
                    //if ( v8 )
                    if (v6 < v10)
                    {
                        RemovePiece(v5, false);
                        //v11 = this->p_this;
                    }
                }
                ++v4;
                //v1 = v11->p_structure;
            } while ( v4 < p_this->p_structure->i_p2p_win_length );
        }
        this->i_piece_offset = p_this->p_structure->i_p2p_win_offset;
    }
}

size_t btCache::RemovePiece(CachePiece *p_piece, bool b_flush) {
    size_t v3; // edi@1
    _cache_piece *v4; // eax@3
    _cache_piece *v5; // edx@5
    CachePiece *v6; // ecx@7
    unsigned int v7; // eax@7
    CachePiece *v8; // ecx@10
    CachePiece *v9; // edx@13
    //goalbit_t_0 *v10; // edx@15
    int v11; // ebp@15
    CachePiece **v12; // ebp@15

    v3 = 0;
    if ( p_piece )
    {
        if ( b_flush && !p_piece->b_flushed )
            FlushPiece(p_piece);
        print_dbg(__func__, "Uncaching piece %d, quality %d", p_piece->i_piece_id, p_piece->i_piece_quality);
        v4 = p_piece->p_newer;
        if ( v4 )
            v4->p_older = p_piece->p_older;
        v5 = p_piece->p_older;
        if ( v5 )
            v5->p_newer = v4;
        v6 = this->p_oldest;
        v7 = p_piece->i_piece_id;
        if ( v6 && v6->i_piece_id == v7 )
            this->p_oldest = p_piece->p_newer;
        v8 = this->p_newest;
        if ( v8 && v8->i_piece_id == v7 )
            this->p_newest = v5;
        v9 = this->p_toflush;
        if ( v9 && v7 == v9->i_piece_id && p_piece->i_piece_quality == v9->i_piece_quality )
        {
            v9->b_flushed = 1;
            while ( 1 )
            {
                v9 = v9->p_newer;
                if ( !v9 )
                    break;
                if ( !v9->b_flushed )
                {
                    this->p_toflush = v9;
                    goto LABEL_15;
                }
            }
            this->p_toflush = 0;
        }
        LABEL_15:
        v3 = p_piece->p_block->i_buffer_size;
        //v10 = this->p_this;
        this->i_cache_used -= v3;
        v11 = v7 % p_this->p_structure->i_p2p_win_length;
        block_Release(p_piece->p_block);
        v12 = &this->p_pieces[v11];
        if ( *v12 == p_piece )
            *v12 = 0;
        free(p_piece);
    }
    return v3;
}

int btCache::WriteSegment(size_t i_quality, size_t i_segment_id, block_t *p_data) {
    //int result; // eax@2

    if ( this->p_files )
        return this->p_files->writeSegment(i_quality, i_segment_id, p_data);
    return -1;
}

block_t* block_Duplicate(block_t *p_block);
int
btCache::AddPiece(block_t *p_data, size_t i_piece_quality, size_t i_piece_id, bool b_should_write) {
    size_t v5; // edx@1
    //unsigned int v6; // eax@1
    size_t v7; // edx@4
    //unsigned int v8; // eax@4
    size_t v9; // esi@7
    //goalbit_structure_t *v10; // ST2C_4@8
    CachePiece *v11; // eax@8
    size_t v12; // eax@10
    size_t v13; // ecx@11
    CachePiece *v14; // eax@13
    CachePiece *v15; // esi@13
    block_t *v16; // eax@13
    CachePiece *v17; // eax@13
    CachePiece **v18; // eax@17
    CachePiece *v19; // ecx@17
    //unsigned int result; // eax@17
    size_t v21; // [sp+20h] [bp-2Ch]@7
    int i_index; // [sp+2Ch] [bp-20h]@8

    v5 = this->i_piece_offset;
    //    v6 = (((signed int)(i_piece_id - v5) >> 0x1F) ^ (i_piece_id - v5)) - ((signed int)(i_piece_id - v5) >> 0x1F);
    //    if ( v6 >= 0x80000000 - v6 )
    //    {
    //        if ( i_piece_id > v5 )
    //            return -1;
    //    }
    //    else if ( i_piece_id < v5 )
    //    {
    //        return -1;
    //    }
    //    if ( v5 == 0x80000001 )
    //        goto LABEL_6;
    v7 = (this->p_this->p_structure->i_p2p_win_length + v5);
    //    v8 = (((signed int)(i_piece_id - v7) >> 0x1F) ^ (i_piece_id - v7)) - ((signed int)(i_piece_id - v7) >> 0x1F);
    //    if ( v8 < 0x80000000 - v8 )
    //    {
    //        if ( i_piece_id <= v7 )
    //            goto LABEL_6;
    //        return -1;
    //    }
    //    if ( i_piece_id < v7 )
    //        return -1;
    //    LABEL_6:
    if (i_piece_id < v5) {
        return -1;
    }
    if (v5 != 0x80000001 && i_piece_id > v7) {
        return -1;
    }
    if ( !p_data ) {
        return -2;
    }
    v9 = this->i_cache_size;
    v21 = p_data->i_buffer_size;
    if ( p_data->i_buffer_size > v9 )
    {
        if ( b_should_write )
        {
            print_dbg(__func__,
                      "[btCache] AddPiece - p_files->writePiece (i_buffer_size: %d, i_cache_size: %d, i_cache_used: %d)",
                      p_data->i_buffer_size,
                      v9,
                      this->i_cache_used);
            return this->p_files->writePiece(i_piece_quality, i_piece_id, p_data);
        }
        return 0;
    }
    //v10 = this->p_this->p_structure;
    v11 = this->p_pieces[i_piece_id % p_this->p_structure->i_p2p_win_length];
    i_index = i_piece_id % p_this->p_structure->i_p2p_win_length;
    if ( v11 )
    {
        RemovePiece(v11, 1);
        v9 = this->i_cache_size;
        v21 = p_data->i_buffer_size;
    }
    v12 = this->i_cache_used;
    if ( v9 - v12 < v21 )
    {
        v13 = v21;
        do
        {
            print_dbg(__func__,
                      "[btCache] AddPiece - RemovePiece (i_buffer_size: %d, i_cache_size: %d, i_cache_used: %d)",
                      v13,
                      v9,
                      v12);
            RemovePiece(this->p_oldest, 1);
            v9 = this->i_cache_size;
            v12 = this->i_cache_used;
            v13 = p_data->i_buffer_size;
        } while ( p_data->i_buffer_size > this->i_cache_size - v12 );
    }
    v14 = new CachePiece();
    v15 = v14;
    v14->i_piece_quality = i_piece_quality;
    v14->i_piece_id = i_piece_id;
    v14->b_flushed = !b_should_write;
    v16 = block_Duplicate(p_data);
    v15->p_newer = 0;
    v15->p_block = v16;
    v17 = this->p_newest;
    v15->p_older = v17;
    if ( v17 )
        v17->p_newer = v15;
    if ( !this->p_oldest )
        this->p_oldest = v15;
    v18 = this->p_pieces;
    this->p_newest = v15;
    v18[i_index] = v15;
    v19 = this->p_toflush;
    this->i_cache_used += p_data->i_buffer_size;
    if ( !v19 && b_should_write )
        this->p_toflush = v15;
    return 0;
}

int btCache::FlushNeeded() {

    int result = 1;
    if ( !this->p_toflush )
        result = this->b_flush_failed;
    return result;
}

void btCache::FlushOldest() {
    print_dbg(__func__, "Flushing oldest unflushed piece from cache");
    if ( this->p_toflush ) {
        FlushPiece(this->p_toflush);
    }
}

block_t *btCache::ReadSegment(size_t i_quality, size_t i_segment_id) {
    if ( this->p_files )
        return this->p_files->readSegment(i_quality, i_segment_id);
    return 0;
}

block_t *btCache::GetPiece(size_t i_piece_quality, size_t i_piece_id) {
    size_t v3; // eax@1
    //unsigned int v4; // edx@1
    //bool v5; // bp@1
    unsigned int v6; // ecx@5
    //unsigned int v7; // eax@5
    //unsigned int v8; // edx@5
    CachePiece *v9; // eax@8
    block_t *v10; // edx@9
    block_t *v11; // eax@11
    block_t *v12; // ebp@11
    _cache_piece *v14; // ecx@17
    _cache_piece *v15; // ecx@19
    CachePiece *v16; // ecx@21
    //bool v17; // [sp+2Fh] [bp-1Dh]@5

    v3 = this->i_piece_offset;
    //    v4 = (((signed int)(i_piece_id - v3) >> 0x1F) ^ (i_piece_id - v3)) - ((signed int)(i_piece_id - v3) >> 0x1F);
    //    v5 = i_piece_id > v3;
    //    if ( v4 < 0x80000000 - v4 )
    //        v5 = i_piece_id < v3;
    //    if ( v5 )
    //        return 0;
    if (i_piece_id < v3) {
        return 0;
    }
    //    if ( v3 == 0x80000001 )
    //    {
    //        v6 = this->p_this->p_structure->i_p2p_win_length;
    //        goto LABEL_8;
    //    }
    v6 = this->p_this->p_structure->i_p2p_win_length;
    //    v7 = (v6 + v3);
    //    v8 = (((signed int)(i_piece_id - v7) >> 0x1F) ^ (i_piece_id - v7)) - ((signed int)(i_piece_id - v7) >> 0x1F);
    //    v17 = v7 > i_piece_id;
    //    if ( v8 < 0x80000000 - v8 )
    //        v17 = v7 < i_piece_id;
    //    if ( v17 )
    //        return 0;
    //    LABEL_8:
    if (v3 != 0x80000001 && i_piece_id > v3+v6) {
        return 0;
    }
    v9 = this->p_pieces[i_piece_id % v6];
    if ( !v9 || (v10 = v9->p_block) == 0 || v9->i_piece_quality != i_piece_quality )
    {
        v11 = this->p_files->readPiece(i_piece_quality, i_piece_id);
        v12 = v11;
        if ( v11 )
            AddPiece(v11, i_piece_quality, i_piece_id, 0);
        return v12;
    }
    if ( this->p_newest != v9 )
    {
        if ( this->p_oldest == v9 )
        {
            this->p_oldest = v9->p_newer;
        }
        else
        {
            v14 = v9->p_older;
            if ( v14 )
                v14->p_newer = v9->p_newer;
        }
        v15 = v9->p_newer;
        if ( v15 )
            v15->p_older = v9->p_older;
        v16 = this->p_newest;
        v16->p_newer = v9;
        v9->p_older = v16;
        v9->p_newer = 0;
        this->p_newest = v9;
    }
    return block_Duplicate(v10);
}

btFiles::btFiles(goalbit_t_0 *p_goalbit) {
    this->p_this = p_goalbit;
    this->i_segment_offset = p_goalbit->p_structure->i_hls_win_offset;
}

int btFiles::writePiece(size_t i_piece_quality, size_t i_piece_id, block_t *p_piece_data) {
    size_t v6; // ebx@1
    //goalbit_structure_t *v7; // edi@2
    size_t v8; // eax@2
    unsigned int v9; // edx@2
    unsigned int v10; // edx@5
    unsigned int v11; // ecx@5
    //int *v12; // eax@7
    //MetadataManager *v13; // eax@7
    int v14; // eax@8
    __int64 v15; // kr00_8@9
    //goalbit_structure_t *v16; // eax@9
    unsigned int v17; // edx@9
    //char *v18; // edi@11
    void *v19; // ebp@15
    unsigned int v20; // eax@15
    int v22; // edi@25
    array_t<metadata_piece_t> *v23; // ebp@25
    int v24; // eax@25
    int i; // ebx@26
    metadata_piece_t *v26; // eax@27
    int v27; // ST24_4@32
    //int v28; // eax@44
    //int v29; // ST08_4@44
    const char *i_current_stream; // [sp+4h] [bp-58h]@40
    int i_segment; // [sp+8h] [bp-54h]@40
    //char *psz_options; // [sp+Ch] [bp-50h]@40
    size_t v33; // [sp+10h] [bp-4Ch]@40
    //int v34; // [sp+14h] [bp-48h]@40
    unsigned int i_segment_id; // [sp+28h] [bp-34h]@8
    FILE *p_file; // [sp+2Ch] [bp-30h]@11
    //int *v37; // [sp+38h] [bp-24h]@7

    v6 = i_piece_id;
    if ( !p_piece_data )
        return 0;
    v8 = p_this->p_structure->i_p2p_win_offset;
    if (i_piece_id < v8 || i_piece_id > v8+p_this->p_structure->i_p2p_win_length) {
        print_err(__func__, "Piece %d is out of range [%d,%d]", i_piece_id, v8, v8+p_this->p_structure->i_p2p_win_length);
        return -1;
    }
   //v12 = _errno_location();
    //v37 = v12;
    //*v12 = 0;
    //v13 = p_this->p_structure->p_metadataManager;
    if ( !p_this->p_structure->p_metadataManager )
        return -1;
    v14 = p_this->p_structure->p_metadataManager->getPieceSegment(i_piece_id);
    i_segment_id = v14;
    if ( v14 == 0x80000001 )
        return -1;
    v15 = this->p_this->p_structure->p_metadataManager->getSegmentSize(i_piece_quality, v14);
    //v16 = this->p_this->p_structure;
    v17 = p_this->p_structure->i_hls_win_offset;
    if ( i_segment_id < v17 || i_segment_id > p_this->p_structure->i_hls_win_length + v17 )
        return -1;
    if (exists_segment_file(i_piece_quality, i_segment_id)) {
        p_file = create_file(i_piece_quality, i_segment_id, "r+b");
    } else {
        p_file = create_file(i_piece_quality, i_segment_id, "w+b");
    }
    if ( !p_file )
    {
        print_err(__func__, "Cannot create file for piece %d, quality %d", i_piece_id, i_piece_quality);
        return -1;
    }
    if ( !get_file_size(p_file) )
    {
        v19 = (void *)malloc(v15);
        memset(v19, 0, v15);
        v20 = fwrite(v19, 1, v15, p_file);
        if ( v15 > v20 )
        {
            print_err(__func__, "Cannot create file for segment %d, quality %d (1)", i_segment_id, i_piece_quality);
            if ( v19 ) {
                free(v19);//operator delete[](v19);
            }
            return -1;
        }
        if ( v19 ) {
            free(v19);//operator delete[](v19);
        }
    }
    v22 = 0;
    v23 = this->p_this->p_structure->p_metadataManager->getSegmentPieces(i_piece_quality, i_segment_id);
    v24 = 0;
    if ( v23 )
    {
        for ( i = 0; i < array_count(v23); ++i )
        {
            v26 = array_item_at_index(v23, i);
            if ( v26 )
            {
                if ( v26->i_piece_id == i_piece_id )
                    break;
                v22 += v26->i_size;
            }
        }
        v6 = i_piece_id;
        v24 = v22;
    }
    else
    {
        v22 = 0;
    }
    v27 = v24;
    freePiecesArray(v23);
    if ( v15 < p_piece_data->i_buffer_index + v27 )
    {
        print_err(__func__, "Cannot insert piece %d into segment %d, quality %d", v6, i_segment_id, i_piece_quality);
        return -1;
    }
    if ( fseek(p_file, v22, 0) < 0 )
    {
        //v28 = strerror(*v37);
        //v29 = v22;
        print_err(__func__, "Failed to seek to %ld on segment %d, quality %d:  %s", errno, i_segment_id, i_piece_quality, strerror(errno));
        return -1;
    }
    if ( fwrite(p_piece_data->p_buffer, 1, p_piece_data->i_buffer_index, p_file) != p_piece_data->i_buffer_index )
    {
        LABEL_41:
        print_err(__func__, "Write failed for piece %d, segment %d, quality %d:  %s", i_segment, i_segment_id, i_piece_quality, strerror(errno));
        fclose(p_file);
        return -1;
    }
    if ( fflush(p_file) == -1 )
    {
        i_segment = v6;
        i_current_stream = "Flush failed for piece %d, segment %d, quality %d:  %s";
        //v34 = strerror(*v37);
        v33 = i_piece_quality;
        //psz_options = (char *)i_segment_id;
        goto LABEL_41;
    }
    fclose(p_file);
    return 0;
}

void btFiles::window_changed() {
    //size_t v1; // edi@1
    //goalbit_t_0 *v2; // edx@2
    //goalbit_t_0 *v3; // ecx@2
    //goalbit_structure_t *v4; // ebx@2
    //size_t v5; // eax@2
    int v6; // esi@6
    char *v8; // eax@7
    size_t i_quality_count; // [sp+4h] [bp-14h]@3
    unsigned int i_segment_id; // [sp+8h] [bp-10h]@4

    //v1 = this->i_segment_offset;
    if ( i_segment_offset == 0x80000001 )
    {
        //v2 = this->p_this;
        //v5 = this->p_this->p_structure->i_hls_win_offset;
        if ( p_this->p_structure->i_hls_win_offset != 0x80000001 )
        {
            this->i_segment_offset = p_this->p_structure->i_hls_win_offset;
            return;
        }
    }
    else
    {
        //v2 = this->p_this;
        //v3 = this->p_this;
        //v4 = this->p_this->p_structure;
        //v5 = p_this->p_structure->i_hls_win_offset;
        if ( i_segment_offset == p_this->p_structure->i_hls_win_offset ) {
            return;
        }
        i_quality_count = p_this->p_structure->i_hls_qualities;
        if ( i_segment_offset < p_this->p_structure->i_hls_win_offset )
        {
            i_segment_id = this->i_segment_offset;
            do
            {
                if ( i_quality_count )
                {
                    v6 = 0;
                    do
                    {
                        v8 = get_file_absolute_path(v6, i_segment_id);
                        if ( v8 )
                        {
                            remove(v8);
                            free(v8);
                        }
                        ++v6;
                    } while ( v6 != i_quality_count );
                    //v3 = this->p_this;
                }
                //v2 = v3;
                i_segment_id++;//v1 = ++i_segment_id;
                //v5 = p_this->p_structure->i_hls_win_offset;
            } while ( p_this->p_structure->i_hls_win_offset > i_segment_id );
        }
    }
    print_msg(__func__, "btFiles - window offset changed from %d to %d", this->i_segment_offset, p_this->p_structure->i_hls_win_offset);
    this->i_segment_offset = this->p_this->p_structure->i_hls_win_offset;
    return;
}

char* btFiles::get_file_absolute_path(int i_segment_quality, int i_segment_id) {
    char *v3; // ebx@1
    char *v4; // eax@2
    char *v5; // edx@3
    char *psz_file_name; // [sp+2Ch] [bp-10h]@1

    v3 = 0;
    psz_file_name = 0;
    if ( asprintf(
            &psz_file_name,
            "%s%c%d_%d.%s",
            this->p_this->p_param->psz_base_dir,
            '/',
            i_segment_quality,
            i_segment_id,
            "ts") > 0 )
    {
        v4 = strdup(psz_file_name);
        v3 = v4;
        if ( *v4 )
        {
            v5 = v4;
            do
                ++v5;
            while ( *v5 );
        }
        free(psz_file_name);
    }
    return v3;
}

unsigned int
btFiles::writeSegment(size_t i_segment_quality, size_t i_segment_id, block_t *p_segment_data) {
    unsigned int v4; // ebp@1
    //int *v5; // eax@2
    //goalbit_structure_t *v6; // eax@2
    size_t v7; // edx@2
    FILE *v9; // esi@4

    v4 = 0;
    if ( p_segment_data )
    {
        //v5 = _errno_location();
        v4 = -1;
        //*v5 = 0;
        //v13 = v5;
        //v6 = this->p_this->p_structure;
        v7 = p_this->p_structure->i_hls_win_offset;
        if ( v7 <= i_segment_id && i_segment_id <= p_this->p_structure->i_hls_win_length + v7 )
        {
            v9 = create_file(i_segment_quality, i_segment_id, "wb");
            if ( !v9 )
            {
                print_err(__func__, "Cannot create file for segment %d, quality %d (2)", i_segment_id, i_segment_quality);
                return v4;
            }
            if ( fwrite(p_segment_data->p_buffer, 1, p_segment_data->i_buffer_index, v9) == p_segment_data->i_buffer_index )
            {
                if ( fflush(v9) != -1 )
                {
                    v4 = 0;
                    fclose(v9);
                    return v4;
                }
                //v12 = strerror(*v13);
                print_err(__func__, "Flush failed for segment %d, quality %d:  %s", i_segment_id, i_segment_quality, strerror(errno));
            }
            else
            {
                //v11 = strerror(*v13);
                print_err(__func__, "Write failed for segment %d, quality %d:  %s", i_segment_id, i_segment_quality, strerror(errno));
            }
            fclose(v9);
            return v4;
        }
    }
    return v4;
}

FILE *btFiles::create_file(size_t i_segment_quality, size_t i_segment_id, const char *psz_options) {
    FILE* v4; // esi@1
    char *v5; // ebx@1

    v4 = 0;
    v5 = get_file_absolute_path(i_segment_quality, i_segment_id);
    if ( v5 )
    {
        v4 = fopen(v5, psz_options);
        free(v5);
    }
    return v4;
}

/**
 * check if a file exists
 * @param i_segment_quality
 * @param i_segment_id
 * @return
 */
int btFiles::exists_segment_file(size_t i_segment_quality, size_t i_segment_id) {
    int v3; // esi@1
    char *v4; // ebx@1
    int v5; // eax@2
    struct stat sb; // [sp+10h] [bp-6Ch]@2

    v3 = 0;
    v4 = get_file_absolute_path(i_segment_quality, i_segment_id);
    if ( v4 )
    {
        v5 = stat(v4, &sb);
        v5 = v5 == 0;
        v3 = v5;
        free(v4);
    }
    return v3;
}

size_t btFiles::get_file_size(FILE *p_file) {
    size_t result; // eax@3
    size_t v3; // esi@4

    if ( p_file && !fseek(p_file, 0, 2) )
    {
        v3 = ftell(p_file);
        rewind(p_file);
        result = v3;
    }
    else
    {
        result = 0;
    }
    return result;
}


/**
 * read segment file
 * @param i_segment_quality
 * @param i_segment_id
 * @return
 */
block_t *btFiles::readSegment(size_t i_segment_quality, size_t i_segment_id) {
    //goalbit_structure_t *v4; // eax@1
    FILE *v6; // eax@3
    FILE *v7; // edi@3
    size_t v9; // eax@8
    size_t v10; // ST2C_4@8
    block_t *v11; // eax@8
    size_t v12; // eax@8
    size_t v13; // edx@8
    block_t *v14; // eax@9
    size_t v15; // ST28_4@10
    int v16; // eax@10
    //int *v17; // eax@11
    //int v18; // eax@11
    block_t *p_chunk; // [sp+3Ch] [bp-20h]@8

    //v4 = this->p_this->p_structure;
    /**
     * i_hls_win_offset ~ i_hls_win_offset+v4->i_hls_win_length
     */
    if ( i_segment_id < p_this->p_structure->i_hls_win_offset || i_segment_id > p_this->p_structure->i_hls_win_offset + p_this->p_structure->i_hls_win_length ) {
        return 0;
    }
    v6 = create_file(i_segment_quality, i_segment_id, "rb");
    v7 = v6;
    if ( !v6 ) {
        print_err(__func__,
                  "Cannot create file for segment %d, quality %d (3)",
                  i_segment_id,
                  i_segment_quality);
        return 0;
    }
    if ( !get_file_size(v6) ) {
        print_err(__func__,
                  "Cannot create file for segment %d, quality %d (3)",
                  i_segment_id,
                  i_segment_quality);
        fclose(v7);
        return 0;
    }
    v9 = get_file_size(v7);
    v10 = v9;
    v11 = block_Alloc(v9);
    p_chunk = v11;
    v12 = fread(v11->p_buffer, 1, v10, v7);
    v13 = v12;
    if ( v10 > v12 && (v15 = v12, v16 = ferror(v7), v13 = v15, v16) )
    {
        //v17 = _errno_location();
        //v18 = strerror(errno);
        print_err(__func__, "Read failed for segment %d, quality %d:  %s", i_segment_id, i_segment_quality, strerror(errno));
        block_Release(p_chunk);
        fclose(v7);
        return 0;
    }
    v14 = p_chunk;
    p_chunk->i_buffer_size = v13;
    v14->i_buffer_index = v13;
    fclose(v7);
    return p_chunk;
}

block_t *btFiles::readPiece(size_t i_piece_quality, size_t i_piece_id) {
    //goalbit_structure_t *v3; // edx@1
    size_t v4; // eax@1
    unsigned int v5; // ecx@1
    unsigned int v6; // edi@4
    unsigned int v7; // ecx@4
    //MetadataManager *v8; // eax@6
    int v9; // eax@7
    uint64_t v10; // kr00_8@8
    //goalbit_structure_t *v11; // eax@8
    unsigned int v12; // edx@8
    FILE *v13; // eax@10
    int v14; // edi@12
    array_t<metadata_piece_t> *v15; // eax@12
    array_t<metadata_piece_t> *v16; // ebp@12
    metadata_piece_t *v17; // eax@14
    int64_t v18; // edx@15
    //unsigned int v19; // ecx@15
    bool v20; // di@18
    block_t *v22; // eax@36
    size_t v23; // eax@36
    size_t v24; // edi@36
    //int *v25; // eax@40
    //int v26; // eax@40
    //int *v27; // eax@42
    //int v28; // eax@42
    //unsigned int v29; // [sp+30h] [bp-4Ch]@19
    int64_t v30; // [sp+34h] [bp-48h]@19
    uint64_t i_offset; // [sp+38h] [bp-44h]@12
    unsigned int i_segment_id; // [sp+40h] [bp-3Ch]@7
    FILE *p_file; // [sp+44h] [bp-38h]@10
    block_t *p_chunk; // [sp+5Ch] [bp-20h]@36

    //v3 = this->p_this->p_structure;
    v4 = p_this->p_structure->i_p2p_win_offset;
    v5 = (((signed int)(i_piece_id - v4) >> 0x1F) ^ (i_piece_id - v4)) - ((signed int)(i_piece_id - v4) >> 0x1F);
    if ( v5 < 0x80000000 - v5 )
    {
        if ( i_piece_id >= v4 )
            goto LABEL_3;
        LABEL_23:
        v6 = (v4 + p_this->p_structure->i_p2p_win_length);
        goto LABEL_24;
    }
    if ( i_piece_id > v4 )
        goto LABEL_23;
    LABEL_3:
    if ( v4 != 0x80000001 )
    {
        v6 = (v4 + p_this->p_structure->i_p2p_win_length);
        v7 = (((signed int)(i_piece_id - v6) >> 0x1F) ^ (i_piece_id - v6)) - ((signed int)(i_piece_id - v6) >> 0x1F);
        if ( v7 >= 0x80000000 - v7 )
        {
            if ( i_piece_id >= v6 )
                goto LABEL_6;
        }
        else if ( i_piece_id <= v6 )
        {
            goto LABEL_6;
        }
        LABEL_24:
        print_err(__func__, "Piece %d is out of range [%d,%d]", i_piece_id, v4, v6);
        return 0;
    }
    LABEL_6:
    //v8 = p_this->p_structure->p_metadataManager;
    if ( !p_this->p_structure->p_metadataManager )
        return 0;
    v9 = p_this->p_structure->p_metadataManager->getPieceSegment(i_piece_id);
    i_segment_id = v9;
    if ( v9 == 0x80000001 )
        return 0;
    v10 = this->p_this->p_structure->p_metadataManager->getSegmentSize(i_piece_quality, v9);
    //v11 = this->p_this->p_structure;
    v12 = p_this->p_structure->i_hls_win_offset;
    if ( i_segment_id < v12 || i_segment_id > p_this->p_structure->i_hls_win_length + v12 )
        return 0;
    v13 = create_file(i_piece_quality, i_segment_id, "rb");
    p_file = v13;
    if ( !v13 )
    {
        print_err(__func__, "Cannot create file for piece %d, quality %d", i_piece_id, i_piece_quality);
        return 0;
    }
    if ( !get_file_size(v13) )
    {
        print_err(__func__, "Cannot create file for piece %d, quality %d", i_piece_id, i_piece_quality);
        fclose(p_file);
        return 0;
    }
    v14 = 0;
    v15 = this->p_this->p_structure->p_metadataManager->getSegmentPieces(i_piece_quality, i_segment_id);
    i_offset = 0LL;
    v16 = v15;
    if ( !v15 || array_count(v15) <= 0 )
    {
        LABEL_18:
        v20 = 1;
        v18 = 0;
        //v19 = 0;
        goto LABEL_19;
    }
    while ( 1 )
    {
        v17 = array_item_at_index(v16, v14);
        if ( v17 )
            break;
        LABEL_17:
        if ( ++v14 >= array_count(v16) )
            goto LABEL_18;
    }
    v18 = v17->i_size;
    //v19 = v17[5];
    if ( v17->i_piece_id != i_piece_id )
    {
        i_offset += v18;//i_offset += __PAIR__(v19, v18);
        goto LABEL_17;
    }
    v20 = v18 == 0;
    LABEL_19:
    v30 = v18;
    //v29 = v19;
    freePiecesArray(v16);
    if ( i_offset > v10
         || v20
         || v10 < v30 + i_offset)
    {
        print_err(__func__,
                  "Cannot read piece %d from segment %d, quality %d", i_piece_id, i_segment_id, i_piece_quality);
        print_err(__func__,
                  "Cannot read piece %d - i_offset = %d, i_segment_size = %d, i_length = %d",
                  i_piece_id,
                  i_offset,
                  v10,
                  v30
        );
        fclose(p_file);
        return 0;
    }
    if ( fseek(p_file, i_offset, 0) < 0 )
    {
        //v27 = _errno_location();
        //v28 = strerror(*v27);
        print_err(__func__,
                  "Failed to seek to %ld on segment %d, quality %d:  %s",
                  i_offset,
                  i_segment_id,
                  i_piece_quality,
                  strerror(errno));
        fclose(p_file);
        return 0;
    }
    v22 = block_Alloc(v30);
    p_chunk = v22;
    v23 = fread(v22->p_buffer, 1, v30, p_file);
    v24 = v23;
    if ( v30 > v23 && ferror(p_file) )
    {
        //v25 = _errno_location();
        //v26 = strerror(*v25);
        print_err(__func__,
                  "Read failed for piece %d, segment %d, quality %d:  %s",
                  i_piece_id,
                  i_segment_id,
                  i_piece_quality,
                  strerror(errno));
        block_Release(p_chunk);
        fclose(p_file);
        return 0;
    }
    p_chunk->i_buffer_index = v24;
    fclose(p_file);
    return p_chunk;
}

block_t* block_Duplicate(block_t *p_block)
{
    block_t *v1; // esi@2
    block_t *v2; // eax@3
    unsigned char *v3; // edx@3
    size_t v4; // ecx@3

    if ( p_block )
    {
        v1 = 0;
        if ( p_block->i_buffer_size )
        {
            v2 = block_Alloc(p_block->i_buffer_size);
            v3 = p_block->p_buffer;
            v4 = p_block->i_buffer_size;
            v1 = v2;
            v2->i_buffer_index = p_block->i_buffer_index;
            memcpy(v2->p_buffer, v3, v4);
        }
    }
    else
    {
        v1 = 0;
    }
    return v1;
}

void btTracker::IntervalCheck() {

    time_t v4; // esi@2
    time_t v11; // eax@20

    block_t *p_responce = 0;
    if ( this->_trackerF46&0b11 ) {
        if ( (this->_trackerF46&0b11) != 1 ) {
            return;//if no send request
        }
        Mypthread_mutex_lock(84, &this->trackercom_lock);
        if ( this->p_tracker_responce )
        {
            p_responce = block_Duplicate(this->p_tracker_responce);
            block_Release(this->p_tracker_responce);
            this->p_tracker_responce = 0;
        }
        Mypthread_mutex_unlock(84, &this->trackercom_lock);
        if ( p_responce )
        {
            this->_trackerF46 = this->_trackerF46 | 0b100u;
            UpdatePeerList(p_responce->p_buffer, p_responce->i_buffer_size);
            block_Release(p_responce);
            Reset(0);
        } else {
            v11 = p_this->p_structure->now - this->m_last_timestamp;
            if ( p_this->p_structure->b_p2p_enabled )
            {
                if ( v11 >= this->m_interval )
                {
                    this->_trackerF46 &= 0b11111100u;
                    return;
                }
            }
            if ( v11 > 4 ) {
                SendRequest();
                time(&this->m_last_timestamp);
            }
        }
    } else {
        v4 = p_this->p_structure->now - this->m_last_timestamp;
        if ( v4 < this->m_interval && p_this->p_structure->WORLD != 0
             && (p_this->p_structure->WORLD->m_peers_count >= p_this->p_param->i_min_peers_list || p_this->p_param->i_min_peers_list > this->m_prevpeers || v4 <= 9)
             && (!(_trackerF46 & 0b1000u) || v4 <= 0) ) {
            if ( p_this->p_structure->now < this->m_last_timestamp ) {
                this->m_last_timestamp = p_this->p_structure->now;
            }
        } else {
            print_dbg(__func__, "[Tracker] - IntervalCheck: Sending request to tracker");
            SendRequest();
            time(&this->m_last_timestamp);
            // set final bit
            _trackerF46 = _trackerF46 & 0b11111100u | 1;
        }
    }
}

static const char *next_key(const char *keylist)
{

    for (; *keylist && *keylist != KEY_SP; keylist++);

    if (*keylist)
        keylist++;

    return keylist;
}

static size_t compare_key(const char *key, size_t keylen, const char *keylist)
{

    for (; keylen && *keylist && *key == *keylist;
           keylen--, key++, keylist++);

    if (!keylen)
        if (*keylist && *keylist != KEY_SP)
            return 1;

    return keylen;
}

size_t buf_long(const char *b, size_t len, char beginchar, char endchar,
                int64_t * pi)
{
    const char *p = b;
    const char *psave;

    if (2 > len)
        return 0; /* buffer too small */

    if (beginchar) {
        if (*p != beginchar)
            return 0;
        p++;
        len--;
    }

    for (psave = p; len && isdigit(*p); p++, len--);

    if (!len || MAX_INT_SIZ < (p - psave) || *p != endchar)
        return 0;

    if (pi) {
        if (beginchar)
            *pi = strtoll(b + 1, (char **) 0, 10);
        else
            *pi = strtoll(b, (char **) 0, 10);
    }
    return(size_t) (p - b + 1);
}

size_t buf_int(const char *b, size_t len, char beginchar, char endchar, size_t * pi)
{

    size_t r;

    if (pi) {
        int64_t pl = 0xdeadbeef; // :)
        r = buf_long(b, len, beginchar, endchar, &pl);
        *pi = (size_t) pl;
    } else {
        r = buf_long(b, len, beginchar, endchar, (int64_t *) 0);
    }
    return r;
}

size_t buf_str(const char *b, size_t len, const char **pstr, size_t * slen)
{

    size_t rl, sl;

    rl = buf_int(b, len, 0, ':', &sl);

    if (!rl)
        return 0;

    if (len < rl + sl)
        return 0u;
    if (pstr)
        *pstr = b + rl;
    if (slen)
        *slen = sl;

    return(rl + sl);
}

size_t decode_int(const char *b, size_t len)
{
    return(buf_long(b, len, 'i', 'e', (int64_t *) 0));
}

size_t decode_str(const char *b, size_t len)
{
    return(buf_str(b, len, (const char **) 0, (size_t *) 0));
}

size_t decode_dict(const char *b, size_t len, const char *keylist)
{
    size_t rl, dl, nl;
    const char *pkey;
    dl = 0;
    if (2 > len || *b != 'd')
        return 0;

    dl++;
    len--;
    for (; len && *(b + dl) != 'e';) {
        rl = buf_str(b + dl, len, &pkey, &nl);

        if (!rl || KEYNAME_SIZ < nl)
            return 0;
        dl += rl;
        len -= rl;

        if (keylist && compare_key(pkey, nl, keylist) == 0) {
            pkey = next_key(keylist);
            if (!*pkey)
                return dl;
            rl = decode_dict(b + dl, len, pkey);
            if (!rl)
                return 0;
            return dl + rl;
        }

        rl = decode_rev(b + dl, len, (const char *) 0);
        if (!rl)
            return 0;

        dl += rl;
        len -= rl;
    }
    if (!len || keylist)
        return 0;
    return dl + 1; /* add the last char 'e' */
}

size_t decode_rev(const char *b, size_t len, const char *keylist)
{
    if (!b) {
        return 0;
    }
    switch (*b) {
        case 'i':
            return decode_int(b, len);
        case 'd':
            return decode_dict(b, len, keylist);
        case 'l':
            return decode_list(b, len, keylist);
        default:
            return decode_str(b, len);
    }
}

size_t decode_list(const char *b, size_t len, const char *keylist)
{
    size_t ll, rl;
    ll = 0;
    if (2 > len || *b != 'l')
        return 0;
    len--;
    ll++;
    for (; len && *(b + ll) != 'e';) {
        rl = decode_rev(b + ll, len, keylist);
        if (!rl)
            return 0;

        ll += rl;
        len -= rl;
    }
    if (!len)
        return 0;
    return ll + 1; /* add last char 'e' */
}

size_t decode_query(const char *b, size_t len, const char *keylist,
                    const char **ps, size_t * pi, int64_t * pl, int method)
{
    size_t pos;
    char kl[KEYNAME_LISTSIZ];
    strcpy(kl, keylist);
    pos = decode_rev(b, len, kl);
    if (!pos)
        return 0;
    switch (method) {
        case QUERY_STR:
            return(buf_str(b + pos, len - pos, ps, pi));
        case QUERY_INT:
            return(buf_int(b + pos, len - pos, 'i', 'e', pi));
        case QUERY_POS:
            if (pi)
                *pi = decode_rev(b + pos, len - pos, (const char *) 0);
            return pos;
        case QUERY_LONG:
            return(buf_long(b + pos, len - pos, 'i', 'e', pl));
        default:
            return 0;
    }
}

size_t bencode_buf(const char *buf, size_t len, FILE * fp)
{
    char slen[MAX_INT_SIZ];

    if (MAX_INT_SIZ <= snprintf(slen, MAX_INT_SIZ, "%d:", (int) len))
        return 0;
    if (fwrite(slen, strlen(slen), 1, fp) != 1)
        return 0;
    if (fwrite(buf, len, 1, fp) != 1)
        return 0;
    return 1;
}

size_t bencode_str(const char *str, FILE * fp)
{
    return bencode_buf(str, strlen(str), fp);
}

size_t bencode_int(const uint64_t integer, FILE * fp)
{
    char buf[MAX_INT_SIZ];
    if (EOF == fputc('i', fp))
        return 0;
    if (MAX_INT_SIZ <=
        snprintf(buf, MAX_INT_SIZ, "%llu", (unsigned long long) integer))
        return 0;
    if (fwrite(buf, strlen(buf), 1, fp) != 1)
        return 0;
    return(EOF == fputc('e', fp)) ? 0 : 1;
}

size_t bencode_begin_dict(FILE * fp)
{
    return(EOF == fputc('d', fp)) ? 0 : 1;
}

size_t bencode_begin_list(FILE * fp)
{
    return(EOF == fputc('l', fp)) ? 0 : 1;
}

size_t bencode_end_dict_list(FILE * fp)
{
    return(EOF == fputc('e', fp)) ? 0 : 1;
}

size_t bencode_path2list(const char *pathname, FILE * fp)
{
    const char *pn;
    const char *p = pathname;

    if (bencode_begin_list(fp) != 1)
        return 0;

    for (; *p;) {
        pn = strchr(p, PATH_SP);
        if (pn) {
            if (bencode_buf(p, pn - p, fp) != 1)
                return 0;
            p = pn + 1;
        } else {
            if (bencode_str(p, fp) != 1)
                return 0;
            break;
        }
    }

    return bencode_end_dict_list(fp);
}

size_t decode_list2path(const char *b, size_t n, char *pathname)
{
    const char *pb = b;
    const char *s = (char *) 0;
    size_t r, q;

    if ('l' != *pb)
        return 0;
    pb++;
    n--;

    if (!n)
        return 0;

    for (; n;) {
        if (!(r = buf_str(pb, n, &s, &q)))
            return 0;
        memcpy(pathname, s, q);
        pathname += q;
        pb += r;
        n -= r;
        if ('e' != *pb) {
            *pathname = PATH_SP, pathname++;
        } else
            break;
    }
    *pathname = '\0';
    return(pb - b + 1);
}

unsigned int btTracker::UpdatePeerList(uint8_t *buf, size_t bufsize)
{
    size_t v6; // eax@7
    size_t v10; // eax@16
    uint8_t *v11; // ecx@18
    char v12; // di@18
    uint8_t *v13; // esi@18
    char v14; // dl@20
    unsigned int v15; // eax@20
    uint8_t *v16; // edi@20
    char v17; // cl@20
    uint8_t *v18; // ecx@24
    unsigned __int16 v20; // di@26
    sockaddr_in ST04_16_27; // ST04_16@27
    bool v24; // zf@30
    size_t v26; // ecx@32
    char *v27; // eax@33
    unsigned int result; // eax@35
    size_t v30; // esi@36
    size_t v33; // eax@40
    array_t<char> *v34; // eax@42
    array_t<char> *v35; // edi@42
    uint8_t *v36; // eax@42
    uint8_t *v37; // ecx@43
    char v39; // bl@46
    size_t v40; // eax@48
    size_t v41; // ebx@51
    uint8_t* v42; // ST24_4@51
    char *v43; // ebp@51
    PeerList *v46; // edx@57
    char *v49; // eax@66
    char *v50; // eax@66
    const char *v52; // eax@66
    int v53; // esi@69
    int len; // ST04_4@70
    void *v55; // eax@70
    size_t cnt; // [sp+28h] [bp-444h]@25
    uint8_t *ps; // [sp+30h] [bp-43Ch]@1
    size_t i; // [sp+34h] [bp-438h]@1
    unsigned int i_peer_score; // [sp+38h] [bp-434h]@26
    char warnmsg[1024]; // [sp+3Ch] [bp-430h]@4
    sockaddr_in addr; // [sp+43Ch] [bp-30h]@1

    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    if ( decode_query((char*)buf, bufsize, "failure reason", (const char**)&ps, &i, 0, 0) )
    {
        if ( i <= 1023 )
        {
            memcpy(warnmsg, ps, i);
            warnmsg[i] = 0;
        }
        else
        {
            memcpy(warnmsg, ps, 1000);
            warnmsg[1000] = 0;
        }
        print_wrn(__func__, "TRACKER FAILURE REASON: %s", warnmsg);
        return -1;
    }
    if ( decode_query((char*)buf, bufsize, "warning message", (const char**)&ps, &i, 0, 0) )
    {
        if ( i > 1023 )
        {
            memcpy(warnmsg, ps, 1000);
            warnmsg[1000] = 0;
        }
        else
        {
            memcpy(warnmsg, ps, i);
            warnmsg[i] = 0;
        }
        print_wrn(__func__, "TRACKER WARNING: %s", warnmsg);
    }
    this->i_peers_count = 0;
    this->i_superpeers_count = 0;
    this->i_broadcasters_count = 0;
    if ( decode_query((char*)buf, bufsize, "tracker id", (const char**)&ps, &i, 0, 0) )
    {
        v30 = i;
        if ( i > 0x14 )
        {
            memcpy(this->m_trackerid, ps, 0x14);
            this->m_trackerid[0x14] = 0;
        }
        else
        {
            memcpy(this->m_trackerid, ps, i);
            this->m_trackerid[v30] = 0;
        }
    }
    v6 = decode_query((char*)buf, bufsize, "interval", 0, &i, 0, 1);
    result = -1;
    if ( !v6 ) {
        return -1;
    }
    if ( this->m_interval != i )
        this->m_interval = i;
    if ( i != this->m_default_interval )
        this->m_default_interval = i;
    if ( decode_query((char*)buf, bufsize, "peer_num", 0, &i, 0, 1) )
    {
        this->i_peers_count = i;
    }
    print_dbg(__func__, "Stats: peers %u", this->i_peers_count);
    if ( decode_query((char*)buf, bufsize, "port_check", 0, &i, 0, 1) )
    {
        this->p_this->p_structure->i_tracker_view_port_opened = i;
        print_dbg(__func__, "Tracker's vision on our opened port: %d", i);
    }
    else
    {
        print_wrn(__func__, "The tracker doesn't give us information on our listening port status");
    }
    v10 = decode_query((char*)buf, bufsize, "peers", 0, 0, 0, 2);
    if ( v10 )
    {
        if ( bufsize - v10 > 3 )
        {
            v11 = &buf[v10];
            v12 = buf[v10];
            ps = &buf[v10];
            addr.sin_family = 2;
            v13 = &buf[bufsize];
            i = 0;
            if ( v12 == 0x3A )
            {
                ps = v11 + 1;
                i = -1;
            }
            else
            {
                if ( v13 <= v11 ) {
                    //goto LABEL_35;
                    return -1;
                }
                v14 = v12;
                v15 = 0;
                v16 = v11;
                v17 = v14;
                while ( 1 )
                {
                    v15 = v17 + 0xA * v15 - 0x30;
                    i = v15;
                    ps = v16 + 1;
                    v17 = v16[1];
                    if ( v17 == 0x3A )
                        break;
                    if ( v16 + 1 == v13 )
                    {
                        return -1;
                    }
                    ++v16;
                }
                v18 = v16 + 2;
                ps = v16 + 2;
                i = v15 / 0xA - 1;
                if ( v15 / 0xA )
                {
                    cnt = 0;
                    do
                    {
                        addr.sin_addr.s_addr = *(_DWORD *)v18;
                        addr.sin_port = *((_WORD *)v18 + 2);
                        i_peer_score = *(_DWORD *)(v18 + 6);
                        print_dbg(__func__, "Received from tracker: %s:%u (%u)", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), i_peer_score);
                        if ( this->p_this->p_structure->Self )
                        {
                            ST04_16_27 = addr;
                            if ( !this->p_this->p_structure->Self->AddrEquiv(ST04_16_27) )
                            {
                                if ( this->p_this->p_structure->IPQUEUE )
                                {
                                    ++cnt;
                                    this->p_this->p_structure->IPQUEUE->Add(&addr, i_peer_score);
                                }
                            }
                        }
                        v18 = ps + 0xA;
                        v24 = i == 0;
                        ps += 0xA;
                        --i;
                    } while ( !v24 );
                    v13 = &buf[bufsize];
                    //v3 = (char*)buf;
                    if ( cnt )
                    {
                        //v25 = this->p_this;
                        this->m_prevpeers = cnt;
                        v26 = cnt;
                        goto LABEL_33;
                    }
                }
            }
            //v25 = this->p_this;
            v46 = this->p_this->p_structure->WORLD;
            if ( v46 )
            {
                v26 = 0;
                this->m_prevpeers = v46->m_peers_count;
            }
            else
            {
                this->m_prevpeers = 0;
                v26 = 0;
            }
            LABEL_33:
            print_dbg(__func__, "new peers=%d; next check in %d sec", v26, this->m_interval);
            v27 = this->p_this->p_structure->psz_p2p_metadata_url;
            if ( v27 )
            {
                LABEL_34:
                return v27 != 0;
                //goto LABEL_35;
            }
            v33 = decode_query((const char*)buf, bufsize, "p2p_urls", 0, 0, 0, 2);
            result = -1;
            if ( v33 && bufsize - v33 > 3 )
            {
                ps = &buf[v33];
                v34 = array_new<char>();
                v35 = v34;
                v36 = ps;
                if ( *ps == 0x6C )
                {
                    v37 = ps++ + 1;
                    if ( v36[1] != 0x65 )
                    {
                        if ( v13 <= v37 ) {
                            //goto LABEL_35;
                            return -1;
                        }
                        //v38 = this;
                        while ( 1 )
                        {
                            i = 0;
                            v39 = *v37;
                            if ( *v37 != 0x3A )
                            {
                                if ( v13 <= v37 ) {
                                    //goto LABEL_53;
                                    return -1;
                                }
                                v40 = 0;
                                while ( 1 )
                                {
                                    ++v37;
                                    v40 = v39 + 0xA * v40 - 0x30;
                                    i = v40;
                                    ps = v37;
                                    v39 = *v37;
                                    if ( *v37 == 0x3A )
                                        break;
                                    if ( v37 == v13 ) {
                                        //goto LABEL_53;
                                        return -1;
                                    }
                                }
                            }
                            v41 = i;
                            //v56 = v38;
                            ps = v37 + 1;
                            v42 = (v37 + 1);
                            v43 = (char *)malloc(i + 1);
                            memcpy(v43, v42, v41);
                            v43[v41] = 0;
                            array_append(v35, v43);
                            v37 = &ps[i];
                            //v38 = v56;
                            ps = v37;
                            if ( *v37 == 0x65 )
                                break;
                            if ( v13 <= v37 ) {
                                //goto LABEL_53;
                                return -1;
                            }
                        }
                        //v4 = v56;
                    }
                }
                if ( array_count(v35) > 0 )
                {
                    v49 = array_item_at_index(v35, 0);
                    p_this->p_structure->psz_p2p_metadata_url = strdup(v49);
                    v52 = p_this->p_structure->psz_p2p_metadata_url;
                    if ( !v52 )
                        v52 = "NULL";
                    print_msg(__func__, "[Tracker] P2P metadata URL set to %s", v52);
                }
                v53 = 0;
                while ( v53 < array_count(v35) )
                {
                    len = v53++;
                    v55 = array_item_at_index(v35, len);
                    free(v55);
                }
                array_destroy(v35);
                v27 = p_this->p_structure->psz_p2p_metadata_url;
                goto LABEL_34;
            }
        }
    }
    return result;
}

void btTracker::Reset(time_t new_interval) {

    if ( new_interval ) {
        this->m_interval = new_interval;
    }
    if ( this->p_this->p_structure->now < this->m_last_timestamp )
        this->m_last_timestamp = this->p_this->p_structure->now;
    if ( _trackerF46 & 0b1000 )
    {
        _trackerF46 = _trackerF46 | 0b11;
        if ( _trackerF46 & 0b100000 )
        {
            _trackerF46 = _trackerF46 & 0b11010000;
            this->m_interval = 15;
        }
    }
    else
    {
        _trackerF46 = _trackerF46 & 0b11111100;
    }
}

void Push_Value(generic_queue_t *queue, generic_event_t event)
{
    queue_node_t *v3; // ebx@1

    v3 = new queue_node_t();
    v3->p_next = 0;
    v3->event.i_event = event.i_event;
    v3->event.value.psz_string = event.value.psz_string;
    Mypthread_mutex_lock(85, &queue->lock);
    if ( queue->p_last )
        queue->p_last->p_next = v3;
    queue->p_last = v3;
    if ( queue->p_first )
    {
        Mypthread_mutex_unlock(85, &queue->lock);
    }
    else
    {
        queue->p_first = v3;
        Mypthread_mutex_unlock(85, &queue->lock);
    }
}

int btTracker::SendRequest() {

    char *v14; // esi@13
    char *v15; // esi@18
    unsigned int v17; // edi@18
    double_t v18; // rax@18
    unsigned long v19; // ecx@22
    int v20; // edi@22
    BasicPeer *v21; // eax@24
    unsigned long v22; // edx@24
    unsigned long v23; // ebp@24
    char *v24; // eax@28
    int v25; // ST50_4@29
    int v26; // ST54_4@29
    generic_event_t ST04_12_31; // ST04_12@31
    int v30; // ST50_4@27
    int v31; // ST54_4@27
    char* v32; // eax@27
    const char *event; // [sp+58h] [bp-94h]@1
    int i_numwant; // [sp+5Ch] [bp-90h]@9
    char *v35; // [sp+60h] [bp-8Ch]@26
    unsigned int i_abi; // [sp+64h] [bp-88h]@5
    unsigned short v38; // [sp+6Ch] [bp-80h]@22
    unsigned int v39; // [sp+70h] [bp-7Ch]@22
    signed int v40; // [sp+74h] [bp-78h]@22
    char *psz_request_url; // [sp+90h] [bp-5Ch]@1
    char *psz_buff_info; // [sp+94h] [bp-58h]@13
    char opt2[32]; // [sp+98h] [bp-54h]@5
    char opt1[20]; // [sp+B8h] [bp-34h]@5

    psz_request_url = 0;
    if ( _trackerF46 & 8 ) {
        event = "stopped";
    } else if (_trackerF46 & 4) {
        event = 0;
    } else {
        event = "started";
    }
    strcpy(opt1, "&event=");
    strcpy(opt2, "&trackerid=");
    Mypthread_mutex_lock(86, &p_this->p_structure->exec_lock);
    i_abi = p_this->p_structure->exec_info.i_abi;
    Mypthread_mutex_unlock(86, &p_this->p_structure->exec_lock);
    if ( p_this->p_structure->WORLD && p_this->p_structure->WORLD->m_peers_count )
    {
        i_numwant = p_this->p_param->i_max_leecher_list + p_this->p_param->i_max_seed_list - p_this->p_structure->IPQUEUE->count;
        if ( p_this->p_param->i_max_leecher_list + p_this->p_param->i_max_seed_list <= p_this->p_structure->IPQUEUE->count )
            i_numwant = 0;
    }
    else
    {
        i_numwant = p_this->p_param->i_max_peers_from_tracker;
    }

    if ( p_this->p_structure->i_external_port > 0 && !this->b_check_port )
    {
        p_this->p_param->i_listen_port = p_this->p_structure->i_external_port;
        p_this->p_structure->i_check_port_opened = 1;
        this->b_check_port = true;
    }
    v14 = byte_8171E59;
    psz_buff_info = 0;
    if ( p_this->p_structure->i_buff_num )
    {
        v14 = 0;
        if ( !this->b_startup_buff_reported )
        {
            asprintf(&psz_buff_info, "&startup_buff_dur=%lu", p_this->p_structure->i_startup_buff_dur);
            //v6 = this->p_this;
            this->b_startup_buff_reported = 1;
            v14 = psz_buff_info;
        }
        if ( p_this->p_structure->i_buff_num <= 1 )
            goto LABEL_20;
        if ( v14 )
        {
            v15 = (char *)strdup(v14);
            free(psz_buff_info);
            //v16 = this->p_this->p_structure;
            v17 = p_this->p_structure->i_buff_num - 1;
            v18 = p_this->p_structure->i_total_rebuff_dur / (unsigned int)v17;
            if ( v15 )
            {
                LABEL_19:
                asprintf(&psz_buff_info, "%s&rebuff_num=%u&rebuff_mean_dur=%lu", v15, v17, (unsigned long)v18);
                //v6 = this->p_this;
                v14 = psz_buff_info;
                //v7 = this->p_this->p_structure;
                LABEL_20:
                if ( !v14 )
                    v14 = byte_8171E59;
                goto LABEL_22;
            }
        }
        else
        {
            v17 = p_this->p_structure->i_buff_num - 1;
            v18 = p_this->p_structure->i_total_rebuff_dur / (unsigned int)v17;
        }
        v15 = byte_8171E59;
        goto LABEL_19;
    }
    LABEL_22:
    v38 = p_this->p_param->i_listen_port;
    v39 = p_this->p_structure->i_target_quality;
    v40 = (signed int)(1000.0 * p_this->p_structure->f_qoe);
    v19 = 0;
    v20 = (_trackerF46 & 0x10u) < 1 ? 0x80000001 : 0;
    if ( p_this->p_structure->p_hlsRate )
    {
        v19 = p_this->p_structure->p_hlsRate->getTotalBytes();
        //v7 = this->p_this->p_structure;
    }
    v21 = p_this->p_structure->Self;
    v22 = 0;
    v23 = 0;
    if ( v21 )
    {
        v22 = v21->p_rate_dl->m_count_bytes;
        v23 = v21->p_rate_ul->m_count_bytes;
    }
    v35 = byte_8171E59;
    if ( this->m_trackerid[0] )
    {
        v30 = v22;
        v31 = v19;
        v32 = strncat(opt2, this->m_trackerid, 0x14u);
        v19 = v31;
        v22 = v30;
        v35 = v32;
    }
    v24 = byte_8171E59;
    if ( event )
    {
        v25 = v22;
        v26 = v19;
        v24 = strcat(opt1, event);
        v19 = v26;
        v22 = v25;
    }
    asprintf(
            &psz_request_url,
            "%s%s%s&p2puploaded=%lu&p2pdownloaded=%lu&hlsdownloaded=%lu&left=%d&compact=1&numwant=%d&abi=%u&qoe=%u&quality=%u&port=%hu&port_check=%d%s",
            this->pzs_base_url,
            v24,
            v35,
            v23,
            v22,
            v19,
            v20,
            i_numwant,
            i_abi,
            v40,
            v39,
            v38,
            p_this->p_structure->i_check_port_opened,
            v14);
    this->p_this->p_structure->i_check_port_opened = 0;
    if ( this->p_server )
    {
        ST04_12_31.i_event = 1;
        ST04_12_31.value.psz_string = strdup(psz_request_url);
        Push_Value(this->p_server->p_events_queue, ST04_12_31);
    }
    if ( psz_buff_info )
        free(psz_buff_info);
    free(psz_request_url);
    return 0;
}

void btTracker::SetStoped() {

    if ( _trackerF46 & 4 )
    {
        Mypthread_mutex_lock(87, &this->trackercom_lock);
        if ( this->p_tracker_responce )
        {
            block_Release(this->p_tracker_responce);
            this->p_tracker_responce = 0;
        }
        Mypthread_mutex_unlock(87, &this->trackercom_lock);
        Reset(15);
        _trackerF46 |= 0b100u;
    }
    else
    {
        _trackerF46 = _trackerF46 | 0b1011u;
    }
}

void* tracker_server_run(void *pvoid)
{
    TrackerServer *p_data = (TrackerServer *)pvoid;
    void *end; // [sp+1Ch] [bp-10h]@2

    p_data->Run();
    pthread_exit(&end);
}

btTracker::btTracker(goalbit_t_0 *p_goalbit) {
    //signed int v2; // ecx@1
    //char *v3; // eax@1
    //unsigned int v4; // edx@3
    //char *v5; // eax@5
    //goalbit_t_0 *v6; // eax@7
    //goalbit_structure_t *v7; // eax@7
    //pthread_t *v10; // eax@7

    //v2 = 0x15;
    //v3 = this->m_trackerid;
    this->p_this = p_goalbit;
    this->p_server = 0;
    this->pzs_base_url = 0;
    //    if ( (int)&this->m_trackerid[0] & 1 )
    //    {
    //        v3 = &this->m_trackerid[1];
    //        v2 = 0x14;
    //        this->m_trackerid[0] = 0;
    //        if ( !((int)&this->m_trackerid[1] & 2) )
    //            goto LABEL_3;
    //        LABEL_14:
    //        *(_WORD *)v3 = 0;
    //        v2 -= 2;
    //        v3 += 2;
    //        goto LABEL_3;
    //    }
    //    if ( (unsigned __int8)v3 & 2 )
    //        goto LABEL_14;
    //    LABEL_3:
    //    v4 = 0;
    //    do
    //    {
    //        *(_DWORD *)&v3[v4] = 0;
    //        v4 += 4;
    //    }
    //    while ( v4 < (v2 & 0xFFFFFFFC) );
    //    v5 = &v3[v4];
    //    if ( !(v2 & 2) )
    //    {
    //        if ( !(v2 & 1) )
    //            goto LABEL_7;
    //        goto LABEL_10;
    //    }
    //    *(_WORD *)v5 = 0;
    //    v5 += 2;
    //    if ( v2 & 1 )
    //        LABEL_10:
    //        *v5 = 0;
    //    LABEL_7:
    memset(m_trackerid, 0, sizeof(m_trackerid));
    //v6 = this->p_this;
    _trackerF46 &= 0b11000000u;
    this->m_default_interval = 0;
    //v7 = v6->p_structure;
    this->m_interval = 15;
    this->i_peers_count = 0;
    this->i_superpeers_count = 0;
    this->i_broadcasters_count = 0;
    this->m_last_timestamp = 0;
    this->m_prevpeers = 0;
    this->m_last_read = p_this->p_structure->now;
    this->b_buff_event = 0;
    this->b_startup_buff_reported = 0;
    this->b_check_port = false;
    pthread_mutex_init(&this->trackercom_lock, 0);
    this->p_tracker_responce = 0;
    this->p_server = new TrackerServer(p_goalbit, this);
    this->p_tracker_server = new pthread_t();
    if ( pthread_create(this->p_tracker_server, 0, tracker_server_run, this->p_server) )
    {
        print_err(goalbitp2p, "pthread_create failed: can't run TrackerServer.");
        this->p_tracker_server = 0;
    }
    return;
}

void btTracker::setTrackerResponse(block_t *p_response) {
    if ( this->p_tracker_responce )
        block_Release(this->p_tracker_responce);
    this->p_tracker_responce = block_Duplicate(p_response);
    return;
}

int btTracker::Initial() {
    int v1; // ebx@1
    int result; // eax@3
    //int v3; // edx@3
    char chars[37]; // [sp+17h] [bp-35h]@1
    //int v5; // [sp+3Ch] [bp-10h]@1

    v1 = 0;
    //v5 = *MK_FP(__GS__, 0x14);
    strcpy(chars, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    do {
        this->m_key[v1++] = chars[random() % 0x24];
    } while ( v1 != 8 );
    this->m_key[8] = 0;
    result = BuildBaseRequest() >> 0x1F;
    //v3 = *MK_FP(__GS__, 0x14) ^ v5;
    return result;
}

char* Http_url_encode(char *s, const unsigned __int8 *b, size_t n)
{
    int v3; // ebx@1
    int v5; // edx@2
    char *v6; // edi@3
    char v7; // bp@3
    char v8; // cl@4
    char *result; // eax@6
    char v11[] = "0123456789ABCDEF"; // [sp+1Bh] [bp-31h]@3
    char v15; // [sp+2Bh] [bp-21h]@3

    v3 = 0;
    if ( n )
    {
        v5 = 0;
        do
        {
            while ( 1 )
            {
                v8 = b[v5];
                ++v5;
                v6 = &s[v3];
                v3 += 3;
                v15 = 0;
                v7 = *(v11 + ((v8 >> 4) & 0xF));
                *v6 = 0x25;
                v6[1] = v7;
                v6[2] = *(v11 + (v8 & 0xF));
                if ( v5 == n )
                    goto LABEL_6;
            }
            s[v3] = v8;
            ++v5;
            ++v3;
        }
        while ( v5 != n );
    }
    LABEL_6:
    result = s;
    s[v3] = 0;
    return result;
}

int btTracker::BuildBaseRequest() {
    char *v1; // eax@1
    //int result; // eax@1
    //int v3; // edx@1
    char pi_buf[61]; // [sp+2Fh] [bp-4Dh]@1
    //int v5; // [sp+6Ch] [bp-10h]@1

    //v5 = *MK_FP(__GS__, 0x14);
    v1 = Http_url_encode(pi_buf, &this->p_this->p_structure->BTCONTENT->m_shake_buffer[45], 20);
    asprintf(
            &this->pzs_base_url,
            "%s?protocol=gbtpd-2.0&content_id=%s&peer_id=%s&key=%s",
            this->p_this->p_param->psz_p2p_tracker_url,
            this->p_this->p_param->psz_content_id,
            v1,
            this->m_key);
    //result = 0;
    //v3 = *MK_FP(__GS__, 0x14) ^ v5;
    return 0;
}

unsigned int _fdelt_warn(unsigned int a1)
{
    //int v1; // ecx@0

    if ( a1 > 0x3FF ) {
        //_chk_fail(v1);
    }
    return a1 >> 5;
}

int soapPostSubmit(int a1, char* a2, char* a3, unsigned short a4, char* a5, const char *a6)
{
    unsigned int v6; // esi@1
    int v7; // ebp@3
    char* v8; // edi@3
    int v10; // [sp+30h] [bp-23Ch]@3
    char v11[0x200]; // [sp+44h] [bp-228h]@3
    char v12[8]; // [sp+244h] [bp-28h]@1
    //int v13; // [sp+24Ch] [bp-20h]@1

    //v13 = *MK_FP(__GS__, 0x14);
    v6 = strlen(a6);
    v12[0] = 0;
    if ( a4 != 0x50 )
        sprintf(v12, ":%hu", a4);
    v7 = sprintf(
            v11,
            "POST %s HTTP/%s\r\n"
                    "Host: %s%s\r\n"
                    "User-Agent: Ubuntu/12.04, UPnP/1.0, MiniUPnPc/1.8\r\n"
                    "Content-Length: %d\r\n"
                    "Content-Type: text/xml\r\n"
                    "SOAPAction: \"%s\"\r\n"
                    "Connection: Close\r\n"
                    "Cache-Control: no-cache\r\n"
                    "Pragma: no-cache\r\n"
                    "\r\n",
            a2, a6, a3, v12, (int)strlen(a5), a5
    );
    v8 = (char*)malloc(v6 + v7);
    v10 = 0;
    if ( v8 )
    {
        memcpy(v8, &v11, v7);
        memcpy(v8 + v7, a6, v6);
        v10 = send(a1, v8, v6 + v7, 0);
        if ( v10 < 0 )
            perror("send");
        free(v8);
    }
    return v10;
}

int parseURL(char *a1, char* a2, u_short * a3, char** a4, int* a5)
{
    int result; // eax@1
    char *v6; // edx@2
    char* v7; // edi@7
    unsigned long long v8; // esi@7
    //_WORD *v9; // edi@11
    char *v10; // esi@16
    char *v11; // eax@16
    char *v12; // edi@16
    char* v13; // eax@20
    int v14; // esi@23
    int v15; // eax@25
    char *v16; // eax@27
    char *v17; // esi@27
    signed int v18; // eax@29
    char *v19; // edi@33
    __int16 v20; // ax@34
    char *v21; // esi@38
    char *v22; // edi@38
    size_t v23; // eax@41
    size_t v24; // eax@44
    char *v25; // esi@47
    __int16 v26; // ax@48
    char *s; // [sp+28h] [bp-44h]@7
    bool v28; // [sp+2Fh] [bp-3Dh]@16
    char v29[16]; // [sp+3Ch] [bp-30h]@25
    //int v30; // [sp+4Ch] [bp-20h]@1

    //    v30 = *MK_FP(__GS__, 0x14);
    result = 0;
    if ( a1 )
    {
        v6 = strstr(a1, "://");
        result = 0;
        if ( v6 )
        {
            if ( *a1 == 0x68 && a1[1] == 0x74 && a1[2] == 0x74 && a1[3] == 0x70 )
            {
                s = v6 + 3;
                v7 = a2;
                v8 = 0x41;
                //                if ( a2 & 1 )
                //                {
                //                    *(_BYTE *)a2 = 0;
                //                    v7 = a2 + 1;
                //                    v8 = 0x40;//LOWORD(v8) = 0x40;
                //                }
                //                if ( v7 & 2 )
                //                {
                //                    *(_WORD *)v7 = 0;
                //                    v7 += 2;
                //                    v8 -= 2;
                //                }
                memset((void *)v7, 0, v8);
                //                v9 = (_WORD *)(v7 + 4 * (v8 >> 2));
                //                if ( v8 & 2 )
                //                {
                //                    *v9 = 0;
                //                    ++v9;
                //                }
                //                if ( v8 & 1 )
                //                    *(_BYTE *)v9 = 0;
                if ( v6[3] != 0x5B )
                    goto LABEL_54;
                v10 = strchr(s, 0x25);
                v11 = strchr(s, 0x5D);
                v12 = v11;
                v28 = v11 != 0;
                if ( v11 && v10 && v10 < v11 && a5 )
                {
                    v13 = (v10 + 1);
                    if ( v10[1] == 0x32 && v10[2] == 0x35 )
                        v13 = (v10 + 3);
                    v14 = v12-v13;
                    if ( v12-v13 > 0xF )
                        v14 = 0xF;
                    memcpy(v29, v13, v14);
                    v29[v14] = 0;
                    v15 = if_nametoindex(v29);
                    *a5 = v15;
                    if ( !v15 )
                        *a5 = strtoul(v29, 0, 0xA);
                }
                v16 = strchr(s, 0x2F);
                v17 = v16;
                if ( v28 && v16 )
                {
                    v18 = v12 + 1 - s;
                    if ( v18 > 0x40 )
                        v18 = 0x40;
                    strncpy((char *)a2, s, v18);
                    if ( v12[1] == 0x3A )
                    {
                        *(_WORD *)a3 = 0;
                        if ( (unsigned __int8)(v12[2] - 0x30) <= 9u )
                        {
                            v19 = v12 + 2;
                            do
                            {
                                v20 = 0xA * *(_WORD *)a3;
                                *(_WORD *)a3 = v20;
                                *(_WORD *)a3 = v20 + *v19++ - 0x30;
                            }
                            while ( (unsigned __int8)(*v19 - 0x30) <= 9u );
                        }
                    }
                    else
                    {
                        *(_WORD *)a3 = 0x50;
                    }
                    *a4 = v17;
                    result = 1;
                }
                else
                {
                    LABEL_54:
                    v21 = strchr(s, 0x3A);
                    v22 = strchr(s, 0x2F);
                    result = 0;
                    if ( v22 )
                    {
                        if ( v21 && v21 <= v22 )
                        {
                            v24 = v21 - s;
                            if ( v21 - s > 0x40 )
                                v24 = 0x40;
                            strncpy((char *)a2, s, v24);
                            *(_WORD *)a3 = 0;
                            if ( (unsigned __int8)(v21[1] - 0x30) <= 9u )
                            {
                                v25 = v21 + 1;
                                do
                                {
                                    v26 = 0xA * *(_WORD *)a3;
                                    *(_WORD *)a3 = v26;
                                    *(_WORD *)a3 = v26 + *v25++ - 0x30;
                                }
                                while ( (unsigned __int8)(*v25 - 0x30) <= 9u );
                            }
                        }
                        else
                        {
                            v23 = v22 - s;
                            if ( v22 - s > 0x40 )
                                v23 = 0x40;
                            strncpy((char *)a2, s, v23);
                            *(_WORD *)a3 = 0x50;
                        }
                        *a4 = v22;
                        result = 1;
                    }
                }
            }
        }
    }
    //    if ( *MK_FP(__GS__, 0x14) != v30 )
    //        _stack_chk_fail_local();
    return result;
}

int receivedata(int a1, void* a2, int a3, int a4, _DWORD *a5)
{
    signed int v5; // eax@2
    int v6; // esi@2
    int result; // eax@11
    pollfd v8; // [sp+30h] [bp-ACh]@2
    socklen_t v9; // [sp+38h] [bp-A4h]@1
    sockaddr v10; // [sp+3Ch] [bp-A0h]@6
    int v11; // [sp+54h] [bp-88h]@10
    //int v12; // [sp+BCh] [bp-20h]@1

    //    v12 = *MK_FP(__GS__, 0x14);
    v9 = 0x80;
    while ( 1 )
    {
        v8.fd = a1;
        v8.events = 1;
        v5 = poll(&v8, 1, a4);
        v6 = v5;
        if ( v5 >= 0 )
            break;
        if ( errno != 4 )
        {
            perror("poll");
            v6 = -1;
            goto LABEL_11;
        }
    }
    if ( v5 )
    {
        v6 = recvfrom(a1, a2, a3, 0, &v10, &v9);
        if ( v6 < 0 )
            perror("recv");
        if ( v10.sa_family == 0xA && a5 )
            *a5 = v11;
    }
    LABEL_11:
    result = v6;
    //    if ( *MK_FP(__GS__, 0x14) != v12 )
    //        _stack_chk_fail_local();
    return result;
}

char* NameValueParserStartElt(char* a1, char* a2, signed int a3)
{
    signed int v3; // esi@1
    char* result; // eax@3

    *(_DWORD *)(a1 + 0x4C) = 1;
    v3 = 0x3F;
    if ( a3 <= 0x3F )
        v3 = a3;
    result = (char*)memcpy(a1 + 4, a2, v3);
    *(_BYTE *)(a1 + v3 + 4) = 0;
    *(_DWORD *)(a1 + 0x50) = 0;
    *(_DWORD *)(a1 + 0x54) = 0;
    return result;
}

int NameValueParserEndElt(char* a1, char* a2, int a3)
{
    bool v1; // cf@1
    bool v2; // zf@1
    _BYTE *v3; // esi@2
    signed int v4; // ecx@2
    const char *v5; // edi@2
    signed int v6; // edi@6
    char* v7; // esi@6
    char* v8; // eax@6
    int v9; // eax@11

    v1 = 0;
    v2 = *(_DWORD *)(a1 + 0x4C) == 0;
    if ( *(_DWORD *)(a1 + 0x4C) )
    {
        v3 = (_BYTE *)(a1 + 4);
        v4 = 0xF;
        v5 = "NewPortListing";
        do
        {
            if ( !v4 )
                break;
            v1 = *v3 < (const unsigned __int8)*v5;
            v2 = *v3++ == *v5++;
            --v4;
        }
        while ( v2 );
        if ( (!v1 && !v2) != v1 )
        {
            v6 = *(_DWORD *)(a1 + 0x54);
            v7 = (char*)malloc(0xC8);
            strncpy((char *)(v7 + 8), (const char *)(a1 + 4), 0x40u);
            *(_BYTE *)(v7 + 0x47) = 0;
            v8 = *(char**)(a1 + 0x50);
            if ( v8 )
            {
                if ( v6 > 0x7F )
                    v6 = 0x7F;
                memcpy(v7 + 0x48, v8, v6);
                *(_BYTE *)(v7 + v6 + 0x48) = 0;
            }
            else
            {
                *(_BYTE *)(v7 + 0x48) = 0;
            }
            v9 = *(_DWORD *)a1;
            *(_DWORD *)v7 = *(_DWORD *)a1;
            if ( v9 )
                *(char**)(*(char**)a1 + 0x10) = v7;
            *(char**)a1 = v7;
            *(char**)(v7 + 4) = a1;
        }
        *(_DWORD *)(a1 + 0x50) = 0;
        *(_DWORD *)(a1 + 0x54) = 0;
        *(_DWORD *)(a1 + 0x4C) = 0;
    }
    return 0;
}

char* NameValueParserGetData(char* a1, char* a2, int a3)
{
    char* result; // eax@2

    if ( !memcmp((const void *)(a1 + 4), "NewPortListing", 0xFu) )
    {
        result = (char*)malloc(a3 + 1);
        *(char**)(a1 + 0x44) = result;
        if ( result )
        {
            memcpy(result, a2, a3);
            result = *(char**)(a1 + 0x44);
            *(_BYTE *)(result + a3) = 0;
            *(_DWORD *)(a1 + 0x48) = a3;
        }
    }
    else
    {
        result = a2;
        *(char**)(a1 + 0x50) = a2;
        *(int*)(a1 + 0x54) = a3;
    }
    return result;
}

void parsexml(xmlst *a1)
{
    xmlst *v1; // esi@1
    char* v2; // eax@1
    char* v3; // ebp@4
    int v4; // ecx@5
    char *result; // eax@5
    char* v6; // edx@9
    //void (*v7)(int, int, int); // eax@16
    char* v8; // ebp@18
    char v9; // cl@19
    int v10; // edx@27
    char* v11; // edi@29
    char *v12; // edi@30
    char v13; // cl@30
    char *v14; // edi@35
    char *v15; // ecx@42
    char* v16; // ecx@42
    char* v17; // edi@44
    char *v18; // edi@45
    char v19; // cl@45
    char* v20; // ebp@53
    char *v21; // ebp@54
    char* v22; // ebp@56
    char *v23; // ebp@57
    char v24; // cl@57
    //void (*v25)(int, _BYTE *, int, char *, int); // ecx@63
    char* v26; // edx@65
    char *v27; // eax@67
    char* v28; // eax@68
    char *v29; // ST24_4@68
    int v30; // ebp@69
    char* v31; // edx@69
    char* v32; // edi@70
    char *v33; // edi@71
    char* v34; // edi@76
    int v35; // ebp@76
    xmlst *v36; // edi@76
    char* v37; // esi@78
    //void (*v38)(int, int, int); // eax@80
    char* v39; // eax@83
    char* v40; // eax@86
    //void (*v41)(_DWORD); // edx@88
    void *s1; // ST00_4@90
    char* v43; // edx@92
    int v44; // ecx@93
    char* v45; // eax@95
    //void (*v46)(int, unsigned int, int); // eax@96
    char* v47; // ebp@100
    char* v48; // edi@100
    int v49; // [sp+20h] [bp-2Ch]@50
    char *v50; // [sp+20h] [bp-2Ch]@68
    char *v51; // [sp+24h] [bp-28h]@51
    char* v52; // [sp+2Ch] [bp-20h]@76

    v1 = a1;
    v2 = (char *)a1->p0;
    a1->p8 = a1->p0;
    a1->p4 = &v2[a1->nC];
    while ( 1 )
    {
        v47 = (char *)v1->p8;
        v48 = (char *)v1->p4;
        result = v48 - 1;
        if ( v47 >= v48-1 )
            return;
        if ( *v47 != 0x3C || v47[1] == 0x3F )
        {
            v1->p8 = (v47 + 1);
        }
        else
        {
            v3 = (v47 + 1);
            v1->p8 = v3;
            LABEL_8:
            v4 = 0;
            while ( 1 )
            {
                v6 = (char *)v1->p8;
                //result = (char *)(unsigned __int8)*v6;
                if ( *v6 == 0x20
                     || *v6 == 9
                     || *v6 == 0xD
                     || *v6 == 0xA
                     || *v6 == 0x3E
                     || *v6 == 0x2F )
                {
                    break;
                }
                ++v4;
                result = v6 + 1;
                v1->p8 = (v6 + 1);
                if ( v6 + 1 >= v48 )
                    return;
                if ( v6[1] == 0x3A )
                {
                    v3 = (v6 + 2);
                    v1->p8 = (v6 + 2);
                    goto LABEL_8;
                }
            }
            if ( v4 <= 0 )
            {
                if ( *v6 == 0x2F )
                {
                    v43 = (v6 + 1);
                    v1->p8 = v43;
                    if ( v43 >= v48 )
                        return;
                    v44 = 0;
                    while ( 1 )
                    {
                        v45 = v1->p8;
                        if ( *(_BYTE *)v45 == 0x3E )
                            break;
                        ++v44;
                        result = (char *)(v45 + 1);
                        v1->p8 = result;
                        if ( result >= v48 )
                            return;
                    }
                    //v46 = (void (__cdecl *)(int, unsigned int, int))v1->f18;
                    if ( v1->f18 )
                        v1->f18(v1->p10, v43, v44);
                    ++v1->p8;
                }
            }
            else
            {
                //v7 = (void (__cdecl *)(int, int, int))v1->f14;
                if ( v1->f14 )
                    v1->f14(v1->p10, v3, v4);
                result = (char *)v1->p8;
                v8 = v1->p4;
                if ( v8 <= result )
                    return;
                v9 = *result;
                if ( *result != 0x2F && v9 != 0x3E )
                {
                    do
                    {
                        if ( v9 != 0x20 && v9 != 9 && v9 != 0xD )
                        {
                            v10 = 0;
                            if ( v9 != 0xA )
                            {
                                while ( 1 )
                                {
                                    v12 = (char *)v1->p8;
                                    v13 = *v12;
                                    if ( *v12 == 0x3D )
                                    {
                                        v14 = result;
                                        goto LABEL_42;
                                    }
                                    if ( v13 == 0x20 )
                                    {
                                        v14 = result;
                                        goto LABEL_42;
                                    }
                                    if ( v13 == 9 )
                                    {
                                        v14 = result;
                                        goto LABEL_42;
                                    }
                                    if ( v13 == 0xD )
                                        break;
                                    if ( v13 == 0xA )
                                    {
                                        v14 = result;
                                        goto LABEL_42;
                                    }
                                    ++v10;
                                    v11 = (v12 + 1);
                                    v1->p8 = v11;
                                    if ( v8 <= v11 )
                                        return;
                                }
                                v14 = result;
                                LABEL_42:
                                while ( 1 )
                                {
                                    v15 = v1->p8;
                                    //result = (char *)*v15;
                                    v16 = (v15 + 1);
                                    v1->p8 = v16;
                                    if ( *v15 == 0x3D )
                                        break;
                                    if ( v1->p4 <= v16 )
                                        return;
                                }
                                result = v14;
                                while ( 1 )
                                {
                                    v18 = (char *)v1->p8;
                                    v19 = *v18;
                                    if ( *v18 != 0x20 && v19 != 9 && v19 != 0xD && v19 != 0xA )
                                        break;
                                    v17 = (v18 + 1);
                                    v1->p8 = v17;
                                    if ( v17 >= v1->p4 )
                                        return;
                                }
                                if ( v19 != 0x27 && (v49 = 0, v19 != 0x22) )
                                {
                                    while ( 1 )
                                    {
                                        v23 = (char *)v1->p8;
                                        v24 = *v23;
                                        if ( *v23 == 0x20 || v24 == 9 || v24 == 0xD || v24 == 0xA || v24 == 0x3E || v24 == 0x2F )
                                            break;
                                        ++v49;
                                        v22 = (v23 + 1);
                                        v1->p8 = v22;
                                        if ( v22 >= v1->p4 )
                                            return;
                                    }
                                }
                                else
                                {
                                    ++v18;
                                    ++v1->p8;
                                    v51 = (char *)v1->p4;
                                    if ( v18 >= v51 )
                                        return;
                                    v49 = 0;
                                    while ( 1 )
                                    {
                                        v21 = v1->p8;
                                        if ( *v21 == v19 )
                                            break;
                                        ++v49;
                                        v20 = (v21 + 1);
                                        v1->p8 = v20;
                                        if ( v51 <= v20 )
                                            return;
                                    }
                                }
                                //v25 = (void (__cdecl *)(int, char *, int, char *, int))v1->n20;
                                if ( v1->f20 )
                                    v1->f20(v1->p10, result, v10, v18, v49);
                            }
                        }
                        v26 = v1->p8;
                        result = (char *)(v26 + 1);
                        v1->p8 = v26 + 1;
                        v8 = v1->p4;
                        if ( v26 + 1 >= v8 )
                            return;
                        v9 = *(_BYTE *)(v26 + 1);
                    }
                    while ( v9 != 0x2F && v9 != 0x3E );
                }
                v27 = v1->p8;
                if ( *v27 != 0x2F )
                {
                    v28 = (v27 + 1);
                    v29 = (char *)v28;
                    v1->p8 = v28;
                    result = (char *)v1->p4;
                    v50 = result;
                    if ( v29 >= result )
                        return;
                    v30 = 0;
                    v31 = v1->p4;
                    while ( 1 )
                    {
                        v33 = (char *)v1->p8;
                        //result = (char *)(unsigned __int8)*v33;
                        if ( *v33 != 0x20 && *v33 != 9 && *v33 != 0xD && *v33 != 0xA )
                            break;
                        ++v30;
                        v32 = (v33 + 1);
                        v1->p8 = v32;
                        if ( v31 <= v32 )
                            return;
                    }
                    if ( !memcmp(v33, "<![CDATA[", 9u) )
                    {
                        v34 = (v33 + 9);
                        v52 = v34;
                        v1->p8 = v34;
                        v35 = 0;
                        v36 = v1;
                        while ( 1 )
                        {
                            v37 = v36->p8;
                            if ( !memcmp((const void *)v36->p8, "]]>", 3u) )
                                break;
                            ++v35;
                            result = (char *)(v37 + 1);
                            v36->p8 = v37 + 1;
                            if ( v50 <= v37 + 4 )
                                return;
                        }
                        v1 = v36;
                        if ( v35 > 0 )
                        {
                            //v38 = (void (__cdecl *)(int, int, int))v36->f1C;
                            if ( v36->f1C )
                                v36->f1C(v36->p10, v52, v35);
                        }
                        while ( 1 )
                        {
                            v39 = v36->p8;
                            if ( *(_BYTE *)v39 == 0x3C )
                                break;
                            result = (char *)(v39 + 1);
                            v36->p8 = result;
                            if ( result >= v36->p4 )
                                return;
                        }
                    }
                    else
                    {
                        while ( 1 )
                        {
                            v40 = v1->p8;
                            if ( *(_BYTE *)v40 == 0x3C )
                                break;
                            ++v30;
                            v1->p8 = v40 + 1;
                            result = (char *)(v40 + 2);
                            if ( v50 <= result )
                                return;
                        }
                        if ( v30 > 0 )
                        {
                            //v41 = (void (__fastcall *)(char *))v1->f1C;
                            if ( v1->f1C )
                            {
                                if ( *(_BYTE *)(v40 + 1) == 0x2F )
                                {
                                    s1 = (void *)v1->p10;
                                    v1->f1C(v1->p10, (char*)"]]>", v30);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return;
}

void Remove_Redirect(goalbit_t_0 *p_this)
{
    //goalbit_structure_t *v1; // eax@1
    int v3; // eax@4
    char eport[6]; // [sp+22h] [bp-1Ah]@4
    char proto[] = "TCP"; // [sp+28h] [bp-14h]@4
    //int v6; // [sp+2Ch] [bp-10h]@1

    if ( p_this->p_structure->p_urls )
    {
        if ( p_this->p_structure->p_data )
        {
            if ( p_this->p_structure->i_external_port )
            {
                sprintf(eport, "%d", p_this->p_structure->i_external_port);
                v3 = UPNP_DeletePortMapping(
                        p_this->p_structure->p_urls->controlURL,
                        p_this->p_structure->p_data->first.servicetype,
                        eport,
                        proto,
                        0);
                print_dbg(__func__, "UPnP: UPNP_DeletePortMapping() returned : %d", v3);
                free(p_this->p_structure->p_urls);
                free(p_this->p_structure->p_data);
            }
        }
    }
}

void PeerList::CloseAll() {
    BT_PEERNODE *v1; // esi@1
    Peer *v2; // edi@2
    bool v3; // zf@2
    BT_PEERNODE *i; // esi@5
    Peer *v5; // edi@6
    pthread_t *v6; // eax@10
    void *end; // [sp+1Ch] [bp-10h]@11

    v1 = this->m_head;
    if ( v1 ) {
        do {
            v2 = v1->peer;
            v3 = v1->peer == 0;
            this->m_head = v1->next;
            if (!v3) {
                //Peer::~Peer(v2);
                //operator delete(v2);
                delete v2;
            }
            //operator delete(v1);
            delete v1;
            v1 = this->m_head;
        } while (v1);
    }
    for ( i = this->m_dead; i; i = this->m_dead )
    {
        v5 = i->peer;
        v3 = i->peer == 0;
        this->m_dead = i->next;
        if ( !v3 )
        {
            //Peer::~Peer(v5);
            //operator delete(v5);
            delete v5;
        }
        //operator delete(i);
        delete i;
    }
    v6 = this->p_upnp_thread;
    if ( v6 )
    {
        pthread_join(*v6, &end);
        free(this->p_upnp_thread);
        this->p_upnp_thread = 0;
    }
    Remove_Redirect(this->p_this);
}

PeerList::PeerList(goalbit_t_0 *p_goalbit) {
    time_t v2; // eax@1
    //goalbit_t_0 *v3; // eax@1
    goalbit_param_t_0 *v4; // edx@1
    size_t v5; // ecx@1
    size_t v6; // edx@1

    this->p_this = p_goalbit;
    this->p_newsocks_queue = Init_Queue();
    pthread_mutex_init(&this->listener_lock, 0);
    this->p_listener_thread = 0;
    this->b_accept_new_conn = false;
    this->p_upnp_thread = 0;
    v2 = time(0);
    this->m_unchoke_interval = 0xA;
    this->m_opt_interval = 0x1E;
    this->m_check_slots_interval = v2;
    this->m_interval_timestamp = v2;
    this->m_opt_timestamp = v2;
    this->m_keepalive_check_timestamp = v2;
    this->m_unchoke_check_timestamp = v2;
    //v3 = this->p_this;
    v4 = this->p_this->p_param;
    v5 = v4->i_max_leecher_unchoke;
    v6 = v4->i_max_seeder_unchoke;
    _peerlistF9e &= 0b1100111;
    this->i_leechers_unchoke = v5;
    this->m_max_unchoke = v5 + v6;
    this->i_seeders_unchoke = v6;
    this->i_opt_unchoke = 0;
    this->m_dead = 0;
    this->m_head = 0;
    this->m_listen_sock = -1;
    this->m_conn_count = 0;
    this->m_leechers_count = 0;
    this->m_seeds_count = 0;
    this->m_peers_count = 0;
    this->m_missed_count = 0;
    this->m_defer_count = 0;
    this->m_upload_count = 0;
    this->m_prev_limit_up = p_this->p_param->i_max_bandwidth_up;
}

void DisplayInfos(goalbit_t_0 *p_this)
{
    int v1; // eax@3
    //int v2; // eax@7
    unsigned int uptime; // [sp+20h] [bp-ECh]@3
    unsigned int brUp; // [sp+24h] [bp-E8h]@3
    unsigned int brDown; // [sp+28h] [bp-E4h]@3
    char connectionType[64]; // [sp+2Ch] [bp-E0h]@1
    char status[64]; // [sp+6Ch] [bp-A0h]@3
    char lastconnerr[64]; // [sp+ACh] [bp-60h]@3
    char externalIPAddress[16]; // [sp+ECh] [bp-20h]@3
    //int v10; // [sp+FCh] [bp-10h]@1

    //v10 = *MK_FP(__GS__, 0x14);
    UPNP_GetConnectionTypeInfo(
            p_this->p_structure->p_urls->controlURL,
            p_this->p_structure->p_data->first.servicetype,
            connectionType);
    if ( connectionType[0] )
        print_dbg(__func__, "UPnP: Connection Type : %s", connectionType);
    else
        print_dbg(__func__, "UPnP: GetConnectionTypeInfo failed.");
    UPNP_GetStatusInfo(
            p_this->p_structure->p_urls->controlURL,
            p_this->p_structure->p_data->first.servicetype,
            status,
            &uptime,
            lastconnerr);
    print_dbg(__func__, "UPnP: Status : %s, uptime=%u, LastConnectionError : %s", status, uptime, lastconnerr);
    UPNP_GetLinkLayerMaxBitRates(
            p_this->p_structure->p_urls->controlURL_CIF,
            p_this->p_structure->p_data->CIF.servicetype,
            &brDown,
            &brUp);
    print_dbg(__func__, "UPnP: MaxBitRateDown : %u bps   MaxBitRateUp %u bps", brDown, brUp);
    v1 = UPNP_GetExternalIPAddress(
            p_this->p_structure->p_urls->controlURL,
            p_this->p_structure->p_data->first.servicetype,
            externalIPAddress);
    if ( v1 )
        print_dbg(__func__, "UPnP: GetExternalIPAddress() returned %d", v1);
    if ( externalIPAddress[0] )
        print_dbg(__func__, "UPnP: ExternalIPAddress = %s", externalIPAddress);
    else
        print_wrn(__func__, "GetExternalIPAddress failed.");
}

void SetRedirectAndTest(goalbit_t_0 *p_this)
{

    int v6; // eax@8
    char externalIPAddress[16]; // [sp+46h] [bp-66h]@1
    char intClient[16]; // [sp+56h] [bp-56h]@8
    char duration[16]; // [sp+66h] [bp-46h]@8
    char intPort[6]; // [sp+76h] [bp-36h]@8
    char iport[6]; // [sp+7Ch] [bp-30h]@3
    char eport[6]; // [sp+82h] [bp-2Ah]@3

    UPNP_GetExternalIPAddress(p_this->p_structure->p_urls->controlURL, p_this->p_structure->p_data->first.servicetype, externalIPAddress);
    if ( externalIPAddress[0] )
        print_dbg(__func__, "UPnP: ExternalIPAddress = %s", externalIPAddress);
    else
        print_dbg(__func__, "UPnP: GetExternalIPAddress failed.");
    char *iaddr = strdup(p_this->p_structure->psz_internal_ip);
    int v3 = p_this->p_param->i_listen_port;
    sprintf(iport, "%d", v3);
    sprintf(eport, "%d", v3);
    int v4 = UPNP_AddPortMapping(
            p_this->p_structure->p_urls->controlURL,
            p_this->p_structure->p_data->first.servicetype,
            eport,
            iport,
            iaddr,
            "GoalbitPlus UPnP",
            "TCP",
            0,
            0);
    while ( v4 == 0x2CE )
    {
        sprintf(eport, "%d", ++v3);
        v4 = UPNP_AddPortMapping(
                p_this->p_structure->p_urls->controlURL,
                p_this->p_structure->p_data->first.servicetype,
                eport,
                iport,
                iaddr,
                "GoalbitPlus UPnP",
                "TCP",
                0,
                0);
    }
    if ( v4 )
        print_wrn(__func__, "AddPortMapping(%s, %s, %s) failed with code %d", eport, iport, iaddr, v4);
    v6 = UPNP_GetSpecificPortMappingEntry(
            p_this->p_structure->p_urls->controlURL,
            p_this->p_structure->p_data->first.servicetype,
            eport,
            "TCP",
            intClient,
            intPort,
            0,
            0,
            duration);
    if ( v6 )
        print_dbg(__func__, "GetSpecificPortMappingEntry() failed with code %d", v6);
    if ( intClient[0] )
    {
        print_dbg(__func__, "UPnP: InternalIP:Port = %s:%s", intClient, intPort);
        print_dbg(goalbitp2p, "UPnP: external %s:%s %s is redirected to internal %s:%s (duration=%s)",
                  externalIPAddress,
                  eport,
                  "TCP",
                  intClient,
                  intPort,
                  duration);
        p_this->p_structure->i_external_port = v3;
    }
}

void Configure_UPnP(goalbit_t_0 *p_this)
{

    int error; // [sp+38h] [bp-34h]@1
    char lanaddr[16]; // [sp+3Ch] [bp-30h]@4

    error = 0;
    UPNPDev *v3 = upnpDiscover(0x7D0u, 0, 0, 0, 0, &error);
    if ( !v3 )
    {
        print_wrn(__func__, "No IGD UPnP Device found on the network !");
        return;
    }
    print_dbg(__func__, "UPnP: List of UPNP devices found on the network :");
    UPNPDev *v31 = v3;
    do
    {
        print_dbg(__func__, "UPnP:  desc: %s\n st: %s", v31->descURL, v31->st);
        v31 = v31->pNext;
    } while ( v31 );
    p_this->p_structure->p_urls = new UPNPUrls();
    p_this->p_structure->p_data = new IGDdatas();
    int v8 = UPNP_GetValidIGD(v3, p_this->p_structure->p_urls, p_this->p_structure->p_data, lanaddr, sizeof(lanaddr));
    if ( !v8 )
    {
        print_wrn(__func__, "No valid UPNP Internet Gateway Device found.");
        freeUPNPDevlist(v3);
        return;
    }
    if ( !strncmp(p_this->p_structure->psz_internal_ip, "0.0.0.0", 8) ) {
        if (!strcmp(p_this->p_structure->psz_internal_ip, lanaddr)) {
            freeUPNPDevlist(v3);
            return;
        }
    }
    switch (v8) {
        case 1:
            print_dbg(__func__, "UPnP: Found valid IGD : %s",
                      p_this->p_structure->p_urls->controlURL);
            break;
        case 2:
            print_dbg(__func__, "UPnP: Found a (not connected?) IGD : %s",
                      p_this->p_structure->p_urls->controlURL);
            print_dbg(__func__, "UPnP: Trying to continue anyway");
            break;
        case 3:
            print_dbg(__func__, "UPnP: UPnP device found. Is it an IGD ? : %s",
                      p_this->p_structure->p_urls->controlURL);
            print_dbg(__func__, "UPnP: Trying to continue anyway");
            break;
        default:
            print_dbg(__func__, "UPnP: Found device (igd ?) : %s",
                      p_this->p_structure->p_urls->controlURL);
            print_dbg(__func__, "UPnP: Trying to continue anyway");
            break;
    }
    strcpy(p_this->p_structure->psz_internal_ip, lanaddr);
    DisplayInfos(p_this);
    SetRedirectAndTest(p_this);
    freeUPNPDevlist(v3);
    return;
}

void* ConfigureUPnPManager(void *p_data)
{
    void *end; // [sp+1Ch] [bp-10h]@2

    Configure_UPnP((goalbit_t_0 *)p_data);
    pthread_exit(&end);
}

void* AccepterRutine(void *pvoid)
{
    goalbit_t_0* p_data = (goalbit_t_0*)pvoid;
    void *end; // [sp+1Ch] [bp-10h]@3

    while (p_data->p_structure->b_btc_alive) {
        p_data->p_structure->WORLD->Accepter();
    }
    pthread_exit(&end);
}

int PeerList::Initial_ListenPort() {
    unsigned __int16 v5; // ax@5
    unsigned int result; // eax@10
    socklen_t addrlen; // [sp+28h] [bp-44h]@21
    sockaddr_in __v; // [sp+2Ch] [bp-40h]@1
    sockaddr_in addr; // [sp+3Ch] [bp-30h]@22

    memset(&__v, 0, sizeof(__v));
    __v.sin_family = AF_INET;
    strcpy(this->m_listen, "n/a");
    this->m_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if ( !this->p_this->p_param->b_listen_for_connections )
    {
        this->p_this->p_param->i_listen_port = random() % 60535 + 5000;
        return -1;
    }
    if ( this->m_listen_sock == -1 )
    {
        return -1;
    }
    if ( this->p_this->p_param->l_listen_ip )
    {
        __v.sin_addr.s_addr = this->p_this->p_param->l_listen_ip;
    } else if ( !__v.sin_addr.s_addr ) {
        addrlen = sizeof(sockaddr_in);
        if ( !getsockname(this->m_listen_sock, (sockaddr*)&addr, &addrlen) )
            __v.sin_addr.s_addr = addr.sin_addr.s_addr;
    }
    if ( !this->p_this->p_param->i_listen_port ) {
        this->p_this->p_param->i_listen_port = p_this->p_structure->cfg_min_listen_port;
    }
    __v.sin_port = htons(this->p_this->p_param->i_listen_port);
    if ( bind(this->m_listen_sock, (sockaddr*)&__v, sizeof(sockaddr)) ) {
        this->p_this->p_param->i_listen_port++;
        if ( this->p_this->p_param->i_listen_port > p_this->p_structure->cfg_max_listen_port )
        {
            shutdown(this->m_listen_sock, 2);
            close(this->m_listen_sock);
            print_wrn(goalbitp2p,
                      "Error, couldn't bind port from %d to %d:  %s",
                      this->p_this->p_structure->cfg_min_listen_port,
                      this->p_this->p_structure->cfg_max_listen_port,
                      strerror(errno));
            return -1;
        }
    }
    if ( listen(this->m_listen_sock, 5) == -1 )
    {
        shutdown(this->m_listen_sock, 2);
        close(this->m_listen_sock);
        print_wrn(goalbitp2p, "Error, couldn't listen on port %hu: %s", this->p_this->p_param->i_listen_port, strerror(errno));
        return -1;
    }
    if ( !this->p_this->p_param->b_disable_upnp )
    {
        strcpy(this->p_this->p_structure->psz_internal_ip, inet_ntoa(__v.sin_addr));
        this->p_upnp_thread = new pthread_t();
        if ( pthread_create(this->p_upnp_thread, 0, ConfigureUPnPManager, p_this) )
        {
            print_dbg(goalbitp2p, "p_upnp_thread failed: p_upnp_thread.");
            this->p_upnp_thread = 0;
            return -1;
        }
    }
    sprintf(this->m_listen, "%s:%d", inet_ntoa(__v.sin_addr), ntohs(__v.sin_port));
    print_wrn(__func__, "Listening on %s", this->m_listen);
    this->p_this->p_structure->Self->SetIp(__v);
    this->p_this->p_structure->Self->SetPort(__v);
    this->p_listener_thread = new pthread_t();
    if ( pthread_create(this->p_listener_thread, 0, AccepterRutine, p_this) )
    {
        print_dbg(goalbitp2p, "pthread_create failed: peer_thread.");
        this->p_listener_thread = 0;
        return -1;
    }
    return 0;
}

/*
 int PeerList::Initial_ListenPort() {
 //char *v1; // ebp@1
 //int v2; // eax@1
 //goalbit_t_0 *v3; // edx@1
 unsigned __int16 v5; // ax@5
 //in_port_t v6; // ax@6
 char* v7; // al@9
 sockaddr_in ST04_16_9; // ST04_16@9
 sockaddr_in ST04_16_9a; // ST04_16@9
 pthread_t *v10; // eax@9
 //goalbit_t_0 *v11; // edx@9
 unsigned int result; // eax@10
 //goalbit_t_0 *v13; // edx@11
 int v14; // eax@14
 //in_port_t v15; // ax@16
 //goalbit_structure_t *v17; // edx@17
 //int *v18; // eax@18
 char* v19; // eax@18
 char* v20; // eax@19
 pthread_t *v21; // eax@19
 //goalbit_t_0 *v22; // edx@19
 //int *v23; // eax@24
 //char* v24; // eax@24
 //goalbit_structure_t *v25; // edx@25
 int v26; // ecx@25
 //int *v27; // eax@29
 char* v28; // eax@29
 socklen_t addrlen; // [sp+28h] [bp-44h]@21
 sockaddr_in __v; // [sp+2Ch] [bp-40h]@1
 sockaddr_in addr; // [sp+3Ch] [bp-30h]@22
 //int v32; // [sp+4Ch] [bp-20h]@1

 //v32 = *MK_FP(__GS__, 0x14);
 //    *(_DWORD *)&__v.sin_family = 0;
 //    __v.sin_addr.s_addr = 0;
 //    *(_DWORD *)&__v.sin_zero[0] = 0;
 //v1 = this->m_listen;
 //    *(_DWORD *)&__v.sin_zero[4] = 0;
 memset(&__v, 0, sizeof(__v));
 __v.sin_family = AF_INET;
 strcpy(this->m_listen, "n/a");
 //v2 = socket(AF_INET, SOCK_STREAM, 0 );
 //v3 = this->p_this;
 this->m_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
 if ( !this->p_this->p_param->b_listen_for_connections )
 {
 this->p_this->p_param->i_listen_port = random() % 60535 + 5000;
 //result = 0xFFFFFFFF;
 //goto LABEL_11;
 return -1;
 }
 if ( this->m_listen_sock == -1 )
 {
 //result = 0xFFFFFFFF;
 //goto LABEL_11;
 return -1;
 }
 if ( this->p_this->p_param->l_listen_ip )
 {
 __v.sin_addr.s_addr = this->p_this->p_param->l_listen_ip;
 v5 = this->p_this->p_param->i_listen_port;
 if ( v5 )
 goto LABEL_6;
 LABEL_14:
 v14 = p_this->p_structure->cfg_min_listen_port;
 goto LABEL_15;
 }
 if ( !__v.sin_addr.s_addr )
 {
 addrlen = sizeof(sockaddr_in);
 if ( !getsockname(this->m_listen_sock, (sockaddr*)&addr, &addrlen) )
 __v.sin_addr.s_addr = addr.sin_addr.s_addr;
 //v3 = this->p_this;
 //v4 = this->p_this->p_param;
 }
 v5 = this->p_this->p_param->i_listen_port;
 if ( !v5 )
 goto LABEL_14;
 LABEL_6:
 //msbtolsb(&v6, &v5, 2);//v6 = __ROR2__(v5, 8);
 //__v.sin_port = v6;
 __v.sin_port = htons(v5);
 if ( bind(this->m_listen_sock, (sockaddr*)&__v, sizeof(sockaddr)) )
 {
 //v23 = errno;
 //v24 = strerror(errno);
 print_wrn(__func__,
 "Warn, couldn't bind on specified port %hu:  %s",
 this->p_this->p_param->i_listen_port,
 strerror(errno)
 );
 //v13 = this->p_this;
 v14 = this->p_this->p_param->i_listen_port;
 if ( v14 )
 {
 //v25 = v13->p_structure;
 v26 = (unsigned __int16)v14 + p_this->p_structure->cfg_max_listen_port - p_this->p_structure->cfg_min_listen_port;
 p_this->p_structure->cfg_min_listen_port = (unsigned __int16)v14;
 p_this->p_structure->cfg_max_listen_port = v26;
 goto LABEL_15;
 }
 goto LABEL_31;
 }
 int _v4;
 while ( 1 )
 {
 if ( listen(this->m_listen_sock, 5) == -1 )
 {
 shutdown(this->m_listen_sock, 2);
 close(this->m_listen_sock);
 //v27 = _errno_location();
 v28 = strerror(errno);
 print_wrn(__func__, "Error, couldn't listen on port %hu: %s", this->p_this->p_param->i_listen_port, v28);
 //result = -1;
 //goto LABEL_11;
 return -1;
 }
 _v4 = __v.sin_port;
 if ( !this->p_this->p_param->b_disable_upnp )
 {
 v20 = inet_ntoa(__v.sin_addr);
 strcpy(this->p_this->p_structure->psz_internal_ip, v20);
 v21 = new pthread_t();
 //v22 = this->p_this;
 this->p_upnp_thread = v21;
 if ( pthread_create(v21, 0, ConfigureUPnPManager, p_this) )
 {
 print_dbg(__func__, "p_upnp_thread failed: p_upnp_thread.");
 //result = -1;
 this->p_upnp_thread = 0;
 //goto LABEL_11;
 return -1;
 }
 _v4 = __v.sin_port;
 }
 //LOWORD(v4) = __ROR2__((_WORD)v4, 8);
 //v4 = (goalbit_param_t_0 *)(unsigned __int16)v4;
 v7 = inet_ntoa(__v.sin_addr);
 sprintf(this->m_listen, "%s:%d", v7, _v4);
 print_msg(__func__, "Listening on %s", this->m_listen);
 //*(_QWORD *)&ST04_16_9.sin_family = *(_QWORD *)&__v.sin_family;
 //*(_QWORD *)&ST04_16_9.sin_zero[0] = *(_QWORD *)&__v.sin_zero[0];
 ST04_16_9 = __v;
 this->p_this->p_structure->Self->SetIp(ST04_16_9);
 //*(_QWORD *)&ST04_16_9a.sin_family = *(_QWORD *)&__v.sin_family;
 //*(_QWORD *)&ST04_16_9a.sin_zero[0] = *(_QWORD *)&__v.sin_zero[0];
 ST04_16_9a = __v;
 this->p_this->p_structure->Self->SetPort(ST04_16_9a);
 v10 = new pthread_t();
 //v11 = this->p_this;
 this->p_listener_thread = v10;
 if ( pthread_create(v10, 0, AccepterRutine, p_this) )
 {
 print_dbg(__func__, "pthread_create failed: peer_thread.");
 //result = -1;
 this->p_listener_thread = 0;
 return -1;
 }
 return 0;
 LABEL_31:
 v14 = p_this->p_structure->cfg_min_listen_port;
 LABEL_15:
 this->p_this->p_param->i_listen_port = v14;
 while ( 1 )
 {
 //msbtolsb(&v15, &v14, 2);//v15 = __ROR2__(v14, 8);
 //__v.sin_port = v15;
 __v.sin_port = htons(v14);
 if ( !bind(this->m_listen_sock, (sockaddr*)&__v, sizeof(sockaddr)) )
 break;
 //v17 = this->p_this->p_structure;
 v14 = this->p_this->p_param->i_listen_port + 1;
 this->p_this->p_param->i_listen_port = v14;
 if ( (unsigned __int16)v14 > p_this->p_structure->cfg_max_listen_port )
 {
 shutdown(this->m_listen_sock, 2);
 close(this->m_listen_sock);
 //v18 = _errno_location();
 v19 = strerror(errno);
 print_wrn(__func__,
 "Error, couldn't bind port from %d to %d:  %s",
 this->p_this->p_structure->cfg_min_listen_port,
 this->p_this->p_structure->cfg_max_listen_port,
 v19);
 //result = 0xFFFFFFFF;
 //goto LABEL_11;
 return -1;
 }
 }
 }
 }
 */

int PeerList::IntervalCheck(fd_set *rfdp, fd_set *wfdp)
{
    int v15; // edx@13
    bool v16; // bp@13
    Peer ** v23; // ebp@23
    BT_PEERNODE *v24; // esi@27
    signed int v25; // edi@28
    int result; // eax@47
    int v39; // esi@51
    int v40; // ebp@51
    size_t v44; // ecx@63
    long double v51; // fst6@73
    const char *when; // [sp+4h] [bp-88h]@65
    signed int when_4; // [sp+8h] [bp-84h]@65
    int f_keepalive_check; // [sp+Ch] [bp-80h]@65
    bool bkeepalive_check; // [sp+30h] [bp-5Ch]@8
    int nLimit; // [sp+30h] [bp-5Ch]@48
    char b_case_1; // [sp+34h] [bp-58h]@13
    int bunchoke_check; // [sp+34h] [bp-58h]@25
    int nLeecherSeed; // [sp+34h] [bp-58h]@48
    unsigned int score; // [sp+58h] [bp-34h]@37
    sockaddr_in addr; // [sp+5Ch] [bp-30h]@37

    if ( this->p_this->p_structure->now - this->m_check_slots_interval > 0xE ) {
        this->m_check_slots_interval = this->p_this->p_structure->now;
        print_wrn(goalbitp2p,
                  "Peer Status (seeders-slots: %d/%d, leechers-slots: %d/%d)",
                  this->m_seeds_count,
                  p_this->p_param->i_max_seed_list,
                  this->m_leechers_count,
                  p_this->p_param->i_max_leecher_list);
        nLeecherSeed = this->m_leechers_count + this->m_seeds_count;
        nLimit = p_this->p_param->i_max_leecher_list + p_this->p_param->i_max_seed_list;
        if ( nLeecherSeed > nLimit )
        {
            if ( this->m_seeds_count <= p_this->p_param->i_max_seed_list )
            {
                if ( this->m_leechers_count > p_this->p_param->i_max_leecher_list ) {
                    v40 = nLeecherSeed - nLimit;
                    print_dbg(goalbitp2p, "Closing connection with %d leechers", v40);
                    CloseConnectionByPeerType(0, v40);
                }
            }
            else if ( this->m_leechers_count > p_this->p_param->i_max_leecher_list ) {
                v39 = this->m_seeds_count - p_this->p_param->i_max_seed_list;
                v40 = this->m_leechers_count - p_this->p_param->i_max_leecher_list;
                print_wrn(goalbitp2p, "Closing connection with %d seeders", v39);
                CloseConnectionByPeerType(1, v39);
                print_wrn(goalbitp2p, "Closing connection with %d leechers", v40);
                CloseConnectionByPeerType(0, v40);
            } else {
                print_wrn(goalbitp2p, "Closing connection with %d seeders", nLeecherSeed - nLimit);
                CloseConnectionByPeerType(1, nLeecherSeed - nLimit);
            }
        }
    }
    if ( this->m_listen_sock != -1 ) {
         if (NeedMorePeers()
             || (8*this->p_this->p_structure->Self->p_rate_dl->RateMeasure()/1000 >= 0.8*p_this->p_structure->f_avg_p2p_bitrate)
                && 8*p_this->p_structure->Self->p_rate_ul->RateMeasure()/1000 >= 1.2*this->p_this->p_structure->f_avg_p2p_bitrate
                && this->m_peers_count < this->p_this->p_param->i_max_leecher_list+this->p_this->p_param->i_max_seed_list*1.5) {
            Mypthread_mutex_lock(88, &this->listener_lock);
            this->b_accept_new_conn = true;
            Mypthread_mutex_unlock(88, &this->listener_lock);
        }
    } else {
        Mypthread_mutex_lock(89, &this->listener_lock);
        this->b_accept_new_conn = false;
        Mypthread_mutex_unlock(89, &this->listener_lock);
    }
    if ( !(p_this->p_structure->Tracker->_trackerF46 & 0b1000) ) {// not stop
        while ( NeedMorePeers() ) {
            if ( !p_this->p_structure->IPQUEUE->count )
                break;
            if ( p_this->p_structure->IPQUEUE->Pop(&addr, &score) < 0 ) {
                break;
            }
            if ( NewPeer(addr, -1, score) == -4 )
                break;
        }
    }
    _peerlistF9e = BandWidthLimitUp(p_this->p_structure->Self->p_rate_ul->m_late) | _peerlistF9e & 0b11111110;
    bkeepalive_check = false;
    if ( this->p_this->p_structure->now - this->m_keepalive_check_timestamp > 0x74 )
    {
        this->m_keepalive_check_timestamp = this->p_this->p_structure->now;
        bkeepalive_check = true;
    }
    if ( this->m_unchoke_interval <= this->p_this->p_structure->now - this->m_unchoke_check_timestamp && this->m_head )
    {
        print_dbg(goalbitp2p,
                  "UL missed %u, sent %u, not_sent: %d, latency: %u",
                  this->m_missed_count,
                  this->m_upload_count,
                  this->m_defer_count,
                  p_this->p_structure->Self->m_latency);
        if ( p_this->p_param->i_max_bandwidth_up )
        {
            if ( p_this->p_param->i_max_leecher_unchoke )
            {
                b_case_1 = 0;
                v15 = p_this->p_param->i_max_seeder_unchoke < 1u ? 1 : 0;
                v16 = p_this->p_param->i_max_seeder_unchoke >= 1u;
            }
            else
            {
                v15 = 0;
                v16 = 0;
                b_case_1 = 1;
            }
        }
        else
        {
            v15 = 0;
            v16 = 0;
            b_case_1 = 0;
        }
        if ( this->m_defer_count > this->m_upload_count && p_this->p_param->i_max_bandwidth_up )
        {
            if ( this->m_max_unchoke > p_this->p_param->i_max_seeder_unchoke + p_this->p_param->i_max_leecher_unchoke ) {
                this->m_max_unchoke = this->m_max_unchoke - 1;
                if (b_case_1 || v16) {
                    this->i_seeders_unchoke--;
                    f_keepalive_check = this->i_seeders_unchoke;
                    when_4 = this->m_max_unchoke;
                    when = "max unchokes down to %u (seeder unchokes: %u)";
                } else {
                    this->i_leechers_unchoke--;
                    f_keepalive_check = this->i_leechers_unchoke;
                    when_4 = this->m_max_unchoke;
                    when = "max unchokes down to %u (leecher unchokes: %u)";
                }
                print_dbg(__func__, when, when_4, f_keepalive_check);
            }
        } else {
            if (this->m_upload_count < this->m_missed_count && p_this->p_param->i_max_bandwidth_up) {
                if (b_case_1 || v16) {
                    if (!this->m_max_unchoke) {
                        this->m_max_unchoke = 1;
                        this->i_seeders_unchoke++;
                        f_keepalive_check = this->i_seeders_unchoke;
                        when_4 = 1;
                        when = "max unchokes up to %u (seeder unchokes: %u)";
                        print_dbg(__func__, when, when_4, f_keepalive_check);
                    }
                } else if (this->m_max_unchoke == GetUnchoked()) {
                    this->m_max_unchoke++;
                    this->i_leechers_unchoke++;
                    print_dbg(goalbitp2p, "max unchokes up to %u (leecher unchokes: %u)", this->m_max_unchoke, this->i_leechers_unchoke);
                }
            }
        }
        this->m_defer_count = 0;
        this->m_upload_count = 0;
        this->m_missed_count = 0;
        if ( !p_this->p_param->i_max_bandwidth_up )
        {
            this->i_leechers_unchoke = p_this->p_param->i_max_leecher_unchoke;
            this->m_max_unchoke = p_this->p_param->i_max_leecher_unchoke + p_this->p_param->i_max_seeder_unchoke;
            this->i_seeders_unchoke = p_this->p_param->i_max_seeder_unchoke;
        }
        this->i_opt_unchoke = 0;
        if ( v15
             && ((long double)(8 * p_this->p_structure->Self->p_rate_dl->RateMeasure() / 1000) >= 0.8 * p_this->p_structure->f_avg_p2p_bitrate)
             && ((long double)(8 * p_this->p_structure->Self->p_rate_ul->RateMeasure() / 1000) >= 1.2 * this->p_this->p_structure->f_avg_p2p_bitrate) )
        {
            print_dbg(goalbitp2p, "we are a good peer, entering in server mode");
            this->i_opt_unchoke = 1;
            if ( (this->i_leechers_unchoke - 4) > 0 )
                this->i_opt_unchoke = this->i_leechers_unchoke - 4;
        }
        else
        {
            if ( this->m_opt_interval && this->m_opt_interval <= p_this->p_structure->now - this->m_opt_timestamp )
            {
                this->i_opt_unchoke = 1;
                this->m_opt_timestamp = 0;
            }
        }
        v23 = (Peer**)malloc(sizeof(Peer*) * this->m_max_unchoke);
        if ( v23 )
            memset(v23, 0, sizeof(Peer*) * this->m_max_unchoke);
        else
            print_wrn(goalbitp2p, "Warn, failed to allocate unchoke array.");
        SetUnchokeIntervals();
        bunchoke_check = 1;
        if ( !this->p_this->p_param->i_cache_size ) {
            return FillFDSet(rfdp, wfdp, bkeepalive_check, bunchoke_check, v23);//goto LABEL_47;
        }
    } else {
        if (this->p_this->p_structure->now < this->m_unchoke_check_timestamp) {
            this->m_unchoke_check_timestamp = this->p_this->p_structure->now;
        }
        if (this->p_this->p_structure->now - this->m_interval_timestamp > 9) {
            this->m_interval_timestamp = this->p_this->p_structure->now;
            if (BandWidthLimitUp(0.0)) {
                if (!this->m_prev_limit_up)
                    goto LABEL_55;
                if ((long double) (p_this->p_param->i_max_bandwidth_up - this->m_prev_limit_up)
                    / (long double) this->m_prev_limit_up <= 1.0 / (long double) this->m_unchoke_interval
                    || (v51 = (this->p_this->p_structure->f_avg_p2p_length +
                               this->p_this->p_structure->f_avg_p2p_length) / 30.0,
                        v51 <= (long double) p_this->p_param->i_max_bandwidth_up)
                       && v51 <= (long double) this->m_prev_limit_up) {
                    v23 = 0;
                    bunchoke_check = 0;
                    goto LABEL_46;
                }
            }
            SetUnchokeIntervals();
            LABEL_55:
            v23 = 0;
            bunchoke_check = 0;
            //goto LABEL_46;
        } else {
            if (this->p_this->p_structure->now < this->m_interval_timestamp)
                this->m_interval_timestamp = this->p_this->p_structure->now;
            v23 = 0;
            bunchoke_check = 0;
        }
        LABEL_46:
        if (!p_this->p_param->i_cache_size) {
            //goto LABEL_47;
            return FillFDSet(rfdp, wfdp, bkeepalive_check, bunchoke_check, v23);
        }
    }
    if ( IsIdle() ) {
        v24 = this->m_head;
        if ( v24 )
        {
            v25 = 1;
            do
            {
                if ( v24->peer->NeedPrefetch() )
                {
                    if ( !v25 && !IsIdle() )
                        break;
                    v25 = 0;
                    v24->peer->Prefetch(this->m_unchoke_interval + this->m_unchoke_check_timestamp);
                }
                v24 = v24->next;
            }
            while ( v24 );
        }
    }
    return FillFDSet(rfdp, wfdp, bkeepalive_check, bunchoke_check, v23);
}

void PeerList::CloseConnectionByPeerType(bool b_seeder, int i_num_to_close)
{
    int v5; // edx@2
    int v12; // edi@16
    Peer* v13; // esi@17
    int v14; // ebp@18
    size_t v15; // eax@18
    Rate *v18; // ecx@22
    long double v19; // fst7@22
    long double v20; // fst7@24
    long double v21; // fst6@24
    Rate* v22; // ecx@26
    long double v23; // fst7@26
    long double v24; // fst6@26
    long double v25; // fst6@28
    long double v26; // fst5@28

    if ( i_num_to_close <= 0 )
        return;
    Peer** v3 = (Peer**)malloc(sizeof(Peer*) * i_num_to_close);
    memset(v3, 0, sizeof(Peer**) * i_num_to_close);
    v5 = 0;

    LABEL_3:
    do {
        for ( BT_PEERNODE *i = this->m_head; i; i = i->next )
        {
            if ( i->peer )
            {
                if ( i->peer != v3[v5] )
                {
                    int v8 = v5;
                    while ( (v8--) != -1 )
                    {
                        if ( i->peer == v3[v8] ) {
                            break;
                        }
                    }
                    if (v8 != -1) {
                        break;
                    }
                    if ( ((i->peer->_peerF28 >> 1) & 0xF) == 3 )
                    {//close
                        v3[v5] = i->peer;
                        ++v5;
                        break;
                    }
                    if ( v3[v5] )
                    {
                        if ( v3[v5]->p_rate_dl->RateMeasure() <= i->peer->p_rate_dl->RateMeasure() )
                        {
                            if ( v3[v5]->p_rate_dl->RateMeasure() != i->peer->p_rate_dl->RateMeasure() )
                                continue;
                            v18 = i->peer->p_rate_ul;
                            v19 = (long double)v18->m_count_bytes;
                            if ( v18->m_count_bytes < 0 )
                                v19 = v19 + 1.8446744e19;
                            v20 = (double)v19;
                            v21 = i->peer->p_rate_dl->m_count_bytes;
                            if ( i->peer->p_rate_dl->m_count_bytes < 0 )
                                v21 = v21 + 1.8446744e19;
                            v22 = v3[v5]->p_rate_ul;
                            v23 = v20 / ((double)v21 + 0.001);
                            v24 = v22->m_count_bytes;
                            if ( v24 < 0 )
                                v24 = v24 + 1.8446744e19;
                            v25 = (double)v24;
                            v26 = v3[v5]->p_rate_dl->m_count_bytes;
                            if ( v3[v5]->p_rate_dl->m_count_bytes < 0 )
                                v26 = v26 + 1.8446744e19;
                            if ( v23 <= v25 / ((double)v26 + 0.001) )
                                continue;
                        }
                    }
                    v3[v5] = i->peer;
                }
            }
        }
    } while (v5 != i_num_to_close);
    LABEL_16:
    v12 = 0;
    do
    {
        v13 = v3[v12];
        if ( v13 )
        {
            v14 = v13->p_rate_ul->m_count_bytes >> 0xA;
            v15 = v13->p_rate_dl->RateMeasure();
            print_dbg(__func__,
                      "Closing connection with peer: %p (download rate from him: %u Bps, uploaded to him: %u KB)",
                      v13,
                      v15,
                      v14);
            v13->CloseConnection();
            v13->_peerF28 &= 0b10111111;//reset 2st bit
        }
        ++v12;
    } while ( v12 != i_num_to_close );
    return;
}

bool PeerList::NeedMorePeers() {

    goalbit_param_t_0 *v2 = this->p_this->p_param;
    if ( this->m_leechers_count >= v2->i_max_leecher_list && this->m_seeds_count >= v2->i_max_seed_list )
    {
        return false;
    }
    if ( this->m_peers_count < v2->i_max_seed_list + v2->i_max_leecher_list )
    {
        return true;
    }
    if ( this->m_peers_count > this->m_seeds_count + this->m_leechers_count ) {
        return false;
    }
    if (8*this->p_this->p_structure->Self->p_rate_dl->RateMeasure()/0x3E8 < 0.8*this->p_this->p_structure->f_avg_p2p_bitrate) {
        return true;
    }
    return false;
}

int PeerList::BandWidthLimitUp(double when) {
    timespec nowspec; // [sp+48h] [bp-24h]@6

    if ( p_this->p_param->i_max_bandwidth_up > 0 )
    {
        long double v5 = p_this->p_structure->Self->p_rate_ul->m_last_size / p_this->p_param->i_max_bandwidth_up + p_this->p_structure->Self->p_rate_ul->m_last_realtime;
        long double v6 = p_this->p_structure->now + when;
        if ( v5 > v6
             || v6 <= v5
                && (clock_gettime(_CLOCK_REALTIME, &nowspec),
                nowspec.tv_sec + nowspec.tv_nsec / 1000000000.0 + when < v5) )
        {
            _peerlistF9e |= 0b10000;
            return 1;
        }
    }
    return 0;
}

size_t PeerList::GetUnchoked() {
    size_t result; // eax@1
    BT_PEERNODE *i; // ecx@1

    result = 0;
    for ( i = this->m_head; i; i = i->next )
    {
        if ( ((i->peer->_peerF28 >> 1) & 0xF) == 2
             && !(i->peer->m_state & 4)
             && ++result > this->m_max_unchoke )
        {
            break;
        }
    }
    return result;
}

void PeerList::SetUnchokeIntervals() {
    time_t v1; // esi@1
    long double v4; // fst6@2
    time_t v5; // edi@2
    time_t v6; // ebp@4
    time_t v7; // ecx@7
    signed int v8; // [sp+38h] [bp-34h]@2
    time_t old_opt_int; // [sp+3Ch] [bp-30h]@1

    v1 = this->m_unchoke_interval;
    old_opt_int = this->m_opt_interval;
    if ( BandWidthLimitUp(0.0) )
    {
        v8 = this->p_this->p_param->i_max_bandwidth_up;
        v4 = p_this->p_structure->f_avg_p2p_length / (long double)v8;
        v5 = (__int64)v4;
        this->m_unchoke_interval = v5;
        if ( v4 - (long double)(signed int)v4 > (float)0.0 )
            this->m_unchoke_interval = ++v5;
        v6 = 3 * v5;
        if ( v5 <= 9 )
        {
            this->m_unchoke_interval = 0xA;
            v6 = 0x1E;
            v5 = 0xA;
        }
        this->m_opt_interval = v6;
    }
    else
    {
        //v2 = this->p_this;
        v5 = 0xA;
        this->m_unchoke_interval = 0xA;
        this->m_opt_interval = 0x1E;
        //v3 = v2->p_structure;
        v8 = p_this->p_param->i_max_bandwidth_up;
    }
    this->m_prev_limit_up = v8;
    this->m_interval_timestamp = p_this->p_structure->now;
    if ( v1 != v5 || this->m_opt_interval != old_opt_int )
        print_dbg(__func__, "ulimit %d, unchoke interval %d, opt interval %d", v8, v5, this->m_opt_interval);
}

int PeerList::IsIdle() {

    signed int v6; // esi@3
    signed int v7; // edi@4
    int result; // eax@4

    if ( p_this->p_param->i_max_bandwidth_down )
    {
        if (p_this->p_structure->now <= p_this->p_structure->Self->p_rate_dl->m_last_realtime
                                        + p_this->p_structure->Self->p_rate_dl->m_late
                                        + p_this->p_structure->Self->p_rate_dl->m_last_size / p_this->p_param->i_max_bandwidth_down) {
            v6 = 0;
            if (!BandWidthLimitDown(p_this->p_structure->Self->p_rate_dl->m_late)) {
                LABEL_21:
                v7 = 0;
                result = 0;
                LABEL_5:
                if (v6)
                    goto LABEL_6;
                goto LABEL_16;
            }
            //goto LABEL_12;
        } else {
            v6 = 1;
            if (_peerlistF9e & 8) {
                v7 = 0;
                result = 0;
                goto LABEL_5;
            }
        }
    } else {
        LABEL_12:
        v6 = 0;
        //goto LABEL_13;
    }
    v7 = 0;
    if ( p_this->p_param->i_max_bandwidth_up )
    {
        v7 = 1;
        if ( p_this->p_structure->now <= (signed int)(p_this->p_structure->Self->p_rate_ul->m_last_realtime + p_this->p_structure->Self->p_rate_ul->m_late + (long double)p_this->p_structure->Self->p_rate_ul->m_last_size / p_this->p_param->i_max_bandwidth_up) )
        {
            v7 = 0;
            if ( !BandWidthLimitUp(p_this->p_structure->Self->p_rate_ul->m_late) )
                goto LABEL_21;
        }
    }
    result = 1;
    if ( v6 )
    {
        LABEL_6:
        if ( _peerlistF9e & 8 )
            result = 0;
        if ( !v7 )
            goto LABEL_9;
        goto LABEL_17;
    }
    LABEL_16:
    _peerlistF9e |= 0b1000;
    if ( !v7 )
    {
        LABEL_9:
        _peerlistF9e |= 0b10000;
        return result;
    }
    LABEL_17:
    if ( _peerlistF9e & 0x10 )
        result = 0;
    return result;
}

int PeerList::BandWidthLimitDown(double when) {
    long double v3; // fst7@1
    signed int v5; // eax@2
    long double v6; // fst6@2
    timespec nowspec; // [sp+48h] [bp-24h]@6

    if ( this->p_this->p_param->i_max_bandwidth_down > 0 ) {
        v6 = p_this->p_structure->Self->p_rate_dl->m_last_size / this->p_this->p_param->i_max_bandwidth_down
             + p_this->p_structure->Self->p_rate_dl->m_last_realtime;
        if ( v6 >= (p_this->p_structure->now + 1) + when
             || p_this->p_structure->now + when <= v6
                && (clock_gettime(_CLOCK_REALTIME, &nowspec),
                (long double)nowspec.tv_sec + (long double)nowspec.tv_nsec / 1000000000.0 + when < (double)v6) )
        {
            _peerlistF9e |= 0b1000;
            return 1;
        }
    }
    return 0;
}

long double PeerList::WaitBW() {
    goalbit_param_t_0 *v1; // edx@1
    long double v4; // fst7@2
    long double v6; // fst6@4
    signed int v7; // edx@5
    double v8; // ST20_8@7
    long double v9; // fst7@7
    long double v10; // fst5@7
    long double v11; // t0@9
    long double v12; // fst5@9
    long double v13; // fst7@9
    BasicPeer *v14; // edx@10
    char v15; // si@10
    signed int v16; // edi@10
    Rate *v17; // eax@10
    long double v18; // fst4@10
    long double v19; // fst3@10
    char v20; // cl@11
    long double v21; // fst3@13
    long double v22; // tt@13
    long double v23; // fst3@13
    long double v24; // t0@13
    long double result; // fst7@15
    long double v26; // t1@18
    long double v27; // fst5@18
    long double v28; // fst6@18
    long double v29; // t2@20
    long double v30; // fst3@21
    long double v31; // fst5@22
    long double v32; // fst4@22
    long double v33; // t2@22
    long double v34; // fst4@22
    long double v35; // fst6@22
    long double v36; // tt@23
    long double v37; // fst4@23
    long double v38; // t0@23
    long double v39; // t1@24
    long double v40; // fst4@24
    long double v41; // t2@24
    long double v42; // tt@27
    long double v43; // fst5@27
    long double v44; // fst6@27
    long double v45; // t0@27
    long double v46; // fst5@27
    long double v47; // fst7@27
    long double v48; // t1@28
    long double v49; // fst5@28
    long double v50; // t2@28
    char v51; // bl@30
    bool v52; // si@32
    long double v53; // t0@35
    long double v54; // fst5@35
    long double v55; // t1@36
    long double v56; // fst3@37
    long double v57; // t0@37
    long double v58; // fst3@37
    long double v59; // t1@37
    timespec nowspec; // [sp+48h] [bp-24h]@7

    v1 = this->p_this->p_param;
    if ( v1->i_max_bandwidth_up )
    {
        v1->i_max_bandwidth_up = p_this->p_structure->Self->p_rate_ul->m_last_size / v1->i_max_bandwidth_up + p_this->p_structure->Self->p_rate_ul->m_last_realtime;
    }
    else
    {
        v4 = 0.0;
    }
    if ( v1->i_max_bandwidth_down )
        v6 = (long double)p_this->p_structure->Self->p_rate_dl->m_last_size / (long double)v1->i_max_bandwidth_down + p_this->p_structure->Self->p_rate_dl->m_last_realtime;
    else
        v6 = 0.0;
    v7 = p_this->p_structure->now;
    if ( v1->i_max_bandwidth_up < v7 && v7 > v6 )
    {
        v10 = v4;
        v9 = (long double)v7;
    }
    else
    {
        v8 = v4;
        clock_gettime(_CLOCK_REALTIME, &nowspec);
        //v2 = this->p_this->p_structure;
        v9 = (long double)nowspec.tv_sec + (long double)nowspec.tv_nsec / 1000000000.0;
        v6 = (double)v6;
        v10 = v8;
    }
    if ( v10 >= v9 )
    {
        v11 = v10;
        v12 = v9;
        v13 = v11;
        if ( v12 > v6 )
        {
            LABEL_10:
            v14 = p_this->p_structure->Self;
            v15 = 0;
            v16 = 1;
            v17 = v14->p_rate_ul;
            v18 = v17->m_late;
            v19 = v13;
            goto LABEL_11;
        }
        v53 = v12;
        v54 = v13;
        v9 = v53;
        if ( v54 <= v6 )
        {
            v55 = v54;
            v12 = v9;
            v13 = v55;
            goto LABEL_10;
        }
        v29 = v54;
        v27 = v6;
        v28 = v29;
        LABEL_21:
        v14 = p_this->p_structure->Self;
        v16 = 0;
        v15 = 1;
        v20 = _peerlistF9e;
        v18 = v14->p_rate_dl->m_late;
        v30 = v27;
        v17 = v14->p_rate_ul;
        if ( !(v20 & 2) )
        {
            v31 = v14->p_rate_dl->m_late;
            v32 = v9;
            v13 = v28;
            v33 = v32;
            v34 = v30;
            v35 = v33;
            goto LABEL_25;
        }
        v21 = v28;
        v6 = v27;
        v22 = v21;
        v23 = v9;
        v13 = v22;
        v24 = v23;
        v19 = v27;
        v12 = v24;
        goto LABEL_14;
    }
    v26 = v10;
    v27 = v6;
    v28 = v26;
    if ( v27 >= v9 )
        goto LABEL_21;
    v14 = p_this->p_structure->Self;
    v18 = 0.0;
    v15 = 0;
    v56 = v28;
    v6 = v27;
    v57 = v56;
    v58 = v9;
    v13 = v57;
    v59 = v58;
    v19 = 0.0;
    v12 = v59;
    v16 = 0;
    v17 = v14->p_rate_ul;
    LABEL_11:
    v20 = _peerlistF9e;
    if ( !(v20 & 2) )
    {
        v36 = v18;
        v37 = v12;
        v31 = v36;
        v38 = v37;
        v34 = v19;
        v35 = v38;
        goto LABEL_25;
    }
    LABEL_14:
    if ( v14->p_rate_dl->m_late + v12 >= v6 )
    {
        LABEL_15:
        v17->_bf40 &= 0b11111110;
        result = -100.0;
        v14->p_rate_dl->_bf40 &= 0b11111110;
        return result;
    }
    v39 = v18;
    v40 = v12;
    v31 = v39;
    v41 = v40;
    v34 = v19;
    v35 = v41;
    LABEL_25:
    if ( v20 & 4 )
    {
        if ( v17->m_late + v35 >= v13 )
            goto LABEL_15;
        v42 = v31;
        v43 = v35;
        v44 = v42;
        v45 = v43;
        v46 = v34;
        v47 = v45;
    }
    else
    {
        v48 = v31;
        v49 = v35;
        v44 = v48;
        v50 = v49;
        v46 = v34;
        v47 = v50;
    }
    if ( v46 <= v47 )
    {
        v17->_bf40 &= 0b11111110;
        result = 0.0;
        v14->p_rate_dl->_bf40 &= 0b11111110;
    }
    else
    {
        v51 = 0;
        result = v46 - v47 - v44;
        v52 = 0;
        if ( result >= 0.0 )
        {
            v51 = v15;
            if ( v16 )
                v52 = 1;
        }
        v17->_bf40 = v52 | v17->_bf40 & 0b11111110;
        v14->p_rate_dl->_bf40 = v51 & 1 | v14->p_rate_dl->_bf40 & 0b11111110;
    }
    return result;
}

void PeerList::AnyNewPeer()
{
    while ( 1 ) {
        queue_node_t *v3 = Pop_Value(this->p_newsocks_queue);
        if ( !v3 )
            break;
        NewPeer(v3->event.value.incoming_connection->addr, v3->event.value.incoming_connection->sk, 0);
        free(v3->event.value.incoming_connection);
        free(v3);
    }
}

void PeerList::AnyPeerReady(fd_set *rfdp, fd_set *wfdp, int *nready, fd_set *rfdnextp,
                            fd_set *wfdnextp) {
    Peer *v7; // esi@3
    signed int v8; // edi@4
    char v9; // al@4
    int v17; // eax@21
    int v18; // eax@22
    int v19; // eax@23
    int v20; // eax@25
    int v21; // eax@27
    BasicPeer *v23; // eax@28
    int v24; // ST1C_4@30
    int v25; // eax@32
    BasicPeer *v27; // eax@33
    int v28; // eax@35
    int v29; // eax@36
    bool v30; // zf@38
    int v31; // eax@38
    BasicPeer *v33; // eax@41
    int v34; // ST1C_4@43
    int v35; // eax@45
    BasicPeer *v37; // eax@47
    int v38; // eax@49
    int v39; // ST20_4@49
    int v40; // eax@49
    int v41; // eax@31
    int v42; // eax@56
    unsigned int v43; // [sp+18h] [bp-34h]@7
    unsigned int v44; // [sp+18h] [bp-34h]@13
    unsigned int v45; // [sp+18h] [bp-34h]@27
    int need_check_send; // [sp+2Ch] [bp-20h]@2

    BT_PEERNODE *v6 = this->m_head;
    if ( v6 ) {
        need_check_send = 0;
        if ( *nready ) {
            do {
                v7 = v6->peer;
                if ( (((v6->peer->_peerF28) >> 1) & 0xF) == 3 ) {
                    continue;
                }
                // not close
                v8 = v7->stream->GetSocket();
                v9 = (v7->_peerF28 >> 1) & 0xF;
                if ( v9 == 2 ) {//send request
                    if (FD_ISSET(v8, rfdp)) {//if ( !_bittest(&v12, v44) )
                        --*nready;
                        if (!(p_this->p_structure->Self->p_rate_ul->_bf40 & 1)) {
                            FD_CLR(v8, rfdnextp);
                            if (v7->ReceivePiece() >= 0) {
                                if (this->p_this->p_structure->Self->p_rate_dl->_bf40 & 1 || v7->HealthCheck() >= 0) {

                                } else {
                                    print_err(goalbitp2p, "Close: unhealthy");
                                    v7->CloseConnection();
                                    FD_CLR(v8, wfdnextp);
                                    continue;
                                }
                            } else {
                                print_err(goalbitp2p, "Close: receive");
                                v7->CloseConnection();
                                FD_CLR(v8, wfdnextp);
                                continue;
                            }
                        }
                    }
                    if (FD_ISSET(v8, wfdp)) {
                        --*nready;
                        if ( !(p_this->p_structure->Self->p_rate_dl->_bf40 & 1) ) {
                            FD_CLR(v8, wfdnextp);
                            need_check_send = 1;
                            if ( v7->SendPiece() < 0 ) {
                                print_err(goalbitp2p, "Close: send");
                                v7->CloseConnection();
                                FD_CLR(v8, rfdnextp);
                            }
                        }
                    } else if ( !(this->p_this->p_structure->Self->p_rate_dl->_bf40 & 1) ) {
                        v30 = v7->CheckSendStatus() == 0;
                        v31 = 0;
                        if ( !v30 )
                            v31 = need_check_send;
                        need_check_send = v31;
                    }
                    continue;//goto LABEL_9;
                }
                if ( v9 == 1 ) {//send handshake
                    if ( FD_ISSET(v8, rfdp) ) {
                        --*nready;
                        v23 = p_this->p_structure->Self;
                        if ( !(v23->p_rate_dl->_bf40 & 1) && !(v23->p_rate_ul->_bf40 & 1) ) {
                            FD_CLR(v8, rfdnextp);
                            if ( v7->ReceiveHandShake() < 0 ) {
                                print_err(__func__, "Close: bad handshake");
                                v7->CloseConnection();
                                FD_CLR(v8, wfdnextp);
                                continue;
                            }
                        }
                    }
                    if (FD_ISSET(v8, wfdp)) {
                        --*nready;
                        v27 = p_this->p_structure->Self;
                        if ( !(v27->p_rate_dl->_bf40 & 1) && !(v27->p_rate_ul->_bf40 & 1) ) {
                            FD_CLR(v8, wfdnextp);
                            if ( v7->SendPiece() < 0 ) {
                                print_err(goalbitp2p, "Close: send handshake");
                                v7->CloseConnection();
                                FD_CLR(v8, rfdnextp);
                                continue;
                            }
                        }
                    }
                } else if ( v9 == 0 ) {
                    if (FD_ISSET(v8, wfdp)) {
                        --*nready;
                        v33 = p_this->p_structure->Self;
                        if ( !(v33->p_rate_dl->_bf40 & 1) && !(v33->p_rate_ul->_bf40 & 1) ) {
                            FD_CLR(v8, wfdnextp);
                            if ( v7->Send_ShakeInfo() < 0 ) {
                                print_err(goalbitp2p, "Close: Sending handshake");
                                v7->CloseConnection();
                                FD_CLR(v8, rfdnextp);
                                continue;
                            }
                            v7->_peerF28 = v7->_peerF28 & 0b11100001 | 2;
                        }
                        if (FD_ISSET(v8, rfdp)) {
                            --*nready;
                        }
                        continue;
                    }
                    if (FD_ISSET(v8, rfdp)) {
                        --*nready;
                        v37 = p_this->p_structure->Self;
                        if ( !(v37->p_rate_dl->_bf40 & 1) && !(v37->p_rate_ul->_bf40 & 1) ) {
                            FD_CLR(v8, rfdnextp);
                            print_err(goalbitp2p, "Close: connect failed");
                            v7->CloseConnection();
                            FD_CLR(v8, wfdnextp);
                        }
                    }
                }
            } while ((v6 = v6->next) && (*nready | need_check_send));
        }
    }
    if ( !(_peerlistF9e & 1) && !BandWidthLimitUp( 0.0) ) {
        ++this->m_missed_count;
    }

}

void PeerList::CheckPeersRequest() {
    print_dbg(__func__, "CheckPeersRequest()");
    for ( BT_PEERNODE *i = this->m_head; i; i = i->next )
    {
        while ( ((i->peer->_peerF28 >> 1) & 0xF) != 2 )
        {
            i = i->next;
            if ( !i )
                return;
        }
        print_dbg(goalbitp2p, "%p->RequestCheck()", i->peer);
        i->peer->RequestCheck();
    }
}

void PeerList::CheckP2PBitFieldFromP2P(btBitField *bf) {
    PIECE *v3; // edi@6

    for ( BT_PEERNODE *i = this->m_head; i; i = i->next )
    {
        while ( 1 )
        {
            if ( ((i->peer->_peerF28 >> 1) & 0xF) == 2 && !i->peer->p_request_q->IsEmpty() )
            {
                v3 = i->peer->p_request_q->GetHead();
                if ( v3 )
                    break;
            }
            i = i->next;
            if ( !i )
                return;
        }
        do
        {
            bf->UnSet(v3->i_quality, v3->i_index);
        }
        while ( v3 = v3->p_next );
    }
}

int PeerList::AlreadyRequested(size_t i_quality, size_t piece_id) {

    BT_PEERNODE *v3 = this->m_head;
    int result = 0;
    if ( v3 )
    {
        do
        {
            if ( ((v3->peer->_peerF28 >> 1) & 0xF) == 2
                 && !v3->peer->p_request_q->IsEmpty()
                 && v3->peer->p_request_q->HasPiece(i_quality, piece_id) )
            {
                return 1;
            }
        }
        while ( v3 = v3->next );
        result = 0;
    }
    return result;
}

int PeerList::AlreadyHaveThisPeer(unsigned __int8 *psz_peer_id) {

    BT_PEERNODE *v2 = this->m_head;
    if (v2 == 0) {
        return 0;
    }
    do {
        if (((v2->peer->_peerF28 >> 1) & 0xF) != 3 && memcmp(v2->peer->id, psz_peer_id, sizeof(v2->peer->id)) == 0) {
            return 1;
        }
    } while (v2 = v2->next);
    return 0;
}

void PeerList::DeleteExpiredRequest(size_t idx) {
    for ( BT_PEERNODE *i = this->m_head; i; i = i->next )
    {
        while ( (((i->peer->_peerF28) >> 1) & 0xF) != 2 )
        {
            i = i->next;
            if ( !i ) {
                this->p_this->p_structure->PENDINGQUEUE->DeletePieceByID(idx);
                return;
            }
        }
        i->peer->CancelPiece(idx);
    }
    this->p_this->p_structure->PENDINGQUEUE->DeletePieceByID(idx);
}

void PeerList::Tell_World_I_Have(size_t i_quality, size_t idx) {

    unsigned int v6; // edx@5
    unsigned int v7; // ecx@7
    unsigned int v8; // edx@7
    unsigned int v10; // edx@9
    size_t v11; // ebp@12
    size_t i_current_quality; // ST2C_4@13

    print_dbg(goalbitp2p, "[PeerList] Tell_World_I_Have(#%u,%u)", idx, i_quality);
    BT_PEERNODE *v3 = this->m_head;
    if ( v3 ) {
        do {
            if ( ((v3->peer->_peerF28 >> 1) & 0x0F) != 2 ) {
                continue;//goto LABEL_3;
            }
            if ( v3->peer->i_win_offset < idx && v3->peer->i_win_length + v3->peer->i_win_offset > idx ) {
                if (idx > v3->peer->i_ab_index) {
                    if (!v3->peer->bitfield->IsSet(i_quality, idx)) {
                        v11 = this->p_this->p_structure->BTCONTENT->GetABIMaxPiece();
                        if (v11 != 0x80000001) {
                            Mypthread_mutex_lock(90, &this->p_this->p_structure->exec_lock);
                            i_current_quality = p_this->p_structure->i_target_quality;
                            Mypthread_mutex_unlock(90, &p_this->p_structure->exec_lock);
                            print_wrn(goalbitp2p,
                                      "[PeerList] Tell_World_I_Have: Send_Have(%u, %u, %u, %u)",
                                      i_quality,
                                      idx,
                                      v11,
                                      i_current_quality);
                            if (v3->peer->stream->Send_Have(i_quality, idx, v11, i_current_quality) < 0) {
                                v3->peer->CloseConnection();
                            }
                        }
                    }
                }
            }
        } while (v3 = v3->next);
        return;
    }
}

void PeerList::CancelPiece(size_t idx) {

    for ( BT_PEERNODE *i = this->m_head; i; i = i->next )
    {
        while ( ((i->peer->_peerF28 >> 1) & 0xF) != 2 || i->peer->CancelPiece(idx) >= 0 )
        {
            i = i->next;
            if ( !i )
                return;
        }
        print_err(__func__, "close: CancelPiece");
        i->peer->CloseConnection();
    }
}

size_t PeerList::GetDownloads() {
    BT_PEERNODE *v1; // ecx@1
    size_t result; // eax@1
    Peer *v3; // ebx@4

    v1 = this->m_head;
    for ( result = 0; v1; result += (v3->m_state & 1u) == 0 )
    {
        while ( 1 )
        {
            v3 = v1->peer;
            if ( ((v1->peer->_peerF28 >> 1) & 0xF) == 2 )
                break;
            v1 = v1->next;
            if ( !v1 )
                return result;
        }
        v1 = v1->next;
    }
    return result;
}

void PeerList::UpdatePeersWindow(size_t i_new_offset) {
    for ( BT_PEERNODE *i = this->m_head; i; i = i->next ) {
        i->peer->bitfield->UpdateWindow(i_new_offset);
        if (((i->peer->_peerF28 >> 1) & 0xF) == 2) {
            if (i->peer->stream->Send_WinUpdate(i_new_offset) < 0) {
                i->peer->CloseConnection();
                continue;
            }
        }
    }
}

void PeerList::Accepter() {

    sockaddr_in __x; // [sp+3Ch] [bp-30h]@2
    socklen_t addrlen = sizeof(sockaddr);
    SOCKET v6 = accept(this->m_listen_sock, (sockaddr*)&__x, &addrlen);
    if ( v6 == -1 )
    {
        print_dbg(__func__, "PeerList - Invalid socket: %d", errno);
        msleep(0x7A120LL);
        return;
    }
    print_wrn(goalbitp2p, "new incoming connection: %s:%hu", inet_ntoa(__x.sin_addr), ntohs(__x.sin_port));
    Mypthread_mutex_lock(91, &this->listener_lock);
    bool b_accept_new_conn = this->b_accept_new_conn;
    Mypthread_mutex_unlock(91, &this->listener_lock);
    if ( __x.sin_family == AF_INET && addrlen == sizeof(sockaddr) && b_accept_new_conn )
    {
        generic_event_t_0 v11; // ST04_12@7
        INCOMING_CONNECTION new_conn; // [sp+28h] [bp-44h]@7
        new_conn.sk = v6;
        new_conn.addr = __x;
        INCOMING_CONNECTION *v10 = new INCOMING_CONNECTION();
        *v10 = new_conn;
        v11.value.incoming_connection = v10;
        v11.i_event = 0;
        Push_Value(this->p_newsocks_queue, v11);
        return;
    }
    shutdown(v6, 2);
    close(v6);
    return;
}

int PeerList::FillFDSet(fd_set *rfdp,
                        fd_set *wfdp,
                        bool bkeepalive_check,
                        bool bunchoke_check,
                        Peer **UNCHOKER)
{
    BT_PEERNODE *prevPeer; // ebp@1
    //goalbit_t_0 *v8; // eax@1
    BT_PEERNODE *v10; // ebx@2
    int v11; // ecx@4
    char v12; // al@5
    unsigned int v14; // ebp@11
    int v15; // eax@11
    int v16; // eax@12
    signed int v19; // esi@16
    unsigned char v20; // al@16
    int v21; // eax@22
    int v22; // eax@22
    size_t v27; // eax@38
    unsigned int v31; // esi@41
    size_t v32; // eax@41
    signed int v33; // esi@41
    int maxfd; // [sp+30h] [bp-4Ch]@1
    bool b_unchoke_somebody; // [sp+37h] [bp-45h]@38
    size_t interested_count; // [sp+38h] [bp-44h]@2
    sockaddr_in addr; // [sp+4Ch] [bp-30h]@59
    //int v54; // [sp+5Ch] [bp-20h]@1

    prevPeer = 0;
    _peerlistF9e = 4 * BandWidthLimitUp(this->p_this->p_structure->Self->p_rate_ul->m_late) | (_peerlistF9e & 0b11111011u);//final 3, 5bit
    _peerlistF9e = 2 * BandWidthLimitDown(p_this->p_structure->Self->p_rate_dl->m_late) | (_peerlistF9e & 0b11111101u);//final 2, 4bit
    maxfd = -1;
    do {
        v10 = this->m_head;
        this->m_seeds_count = 0;
        this->m_leechers_count = 0;
        this->m_conn_count = 0;
        interested_count = 0;
        if ( v10 )
        {
            do {
                v19 = v10->peer->stream->GetSocket();
                v20 = (v10->peer->_peerF28 >> 1) & 0b1111u;
                if ( v20 == 3 ) {//close
                    if ( v19 != -1 )
                    {
                        FD_CLR(v19, rfdp);
                        FD_CLR(v19, wfdp);
                    }
                    if ( (v10->peer->_peerF28 & 0x0C0) == 0xC0)
                    {
                        print_dbg(goalbitp2p, "Adding %p for reconnect", v10->peer);
                        v10->peer->_bf29 |= 1u;
                        addr.sin_family = v10->peer->m_sin.sin_family;
                        addr.sin_port = v10->peer->m_sin.sin_port;
                        addr.sin_addr = v10->peer->m_sin.sin_addr;
                        this->p_this->p_structure->IPQUEUE->Add(&addr, v10->peer->score);
                    }
                    if (!prevPeer) {
                        this->m_head = v10->next;
                    } else {
                        prevPeer->next = v10->next;
                    }
                    if ( v10->peer->p_rate_dl->m_count_bytes || v10->peer->p_rate_ul->m_count_bytes )
                    {
                        time(&v10->peer->m_last_timestamp);
                        v10->next = this->m_dead;
                        this->m_dead = v10;
                    }
                    else
                    {
                        delete v10->peer;
                    }
                    --this->m_peers_count;
                    if ( !prevPeer ) {
                        v10 = this->m_head;
                    } else {
                        v10 = prevPeer->next;
                    }
                } else {
                    if (v20 == 2) {//send request
                        ++this->m_leechers_count;
                        if (v10->peer->m_state & 0b1000u) {
                            interested_count++;
                        }
                    } else {
                        ++this->m_conn_count;
                    }
                    if (bkeepalive_check) {
                        v11 = this->p_this->p_structure->now - v10->peer->m_last_timestamp;
                        if (v11 > 0x15E) {
                            print_err(goalbitp2p, "close: keepalive expired");
                            v10->peer->CloseConnection();
                            if (((v10->peer->_peerF28 >> 1) & 0b1111u) == 3) {
                                FD_CLR(v19, rfdp);
                                FD_CLR(v19, wfdp);
                            }
                            prevPeer = v10;
                            v10 = v10->next;
                            continue;
                        }
                        v12 = (v10->peer->_peerF28 >> 1) & 0b1111u;
                        if (v12 == 2 && v11 > 0x74) {
                            if (v10->peer->AreYouOK() < 0) {
                                print_err(goalbitp2p, "close: keepalive death");
                                v10->peer->CloseConnection();
                                prevPeer = v10;
                                v10 = v10->next;
                                continue;
                            }
                        }
                    }
                    v12 = (v10->peer->_peerF28 >> 1) & 0b1111u;
                    if (bunchoke_check) {
                        if (v12 == 2) {
                            if (v10->peer->m_state & 2) {
                                if (v10->peer->Need_Local_Data() && UNCHOKER &&
                                    UnChokeCheck(v10->peer, UNCHOKER) < 0) {
                                    prevPeer = v10;
                                    v10 = v10->next;
                                    continue;

                                }
                            }
                        } else if (v12 == 3) {//close
                            FD_CLR(v19, rfdp);
                            FD_CLR(v19, wfdp);
                            prevPeer = v10;
                            v10 = v10->next;
                            continue;
                        }
                    }
                    if (maxfd < v19)
                        maxfd = v19;
                    if (!FD_ISSET(v19, rfdp) && v10->peer->NeedRead((_peerlistF9e >> 1) & 1))//if ( !_bittest(&v15, v14) && v10->peer->NeedRead((_peerlistF9e >> 1) & 1) )
                    {
                        FD_SET(v19, rfdp);
                    }
                    if (!FD_ISSET(v19, wfdp) && v10->peer->NeedWrite((_peerlistF9e >> 2) & 1))//if ( !_bittest(&v16, v14) && v10->peer->NeedWrite((_peerlistF9e >> 2) & 1) )
                    {
                        FD_SET(v19, wfdp);
                    }
                    if (((v10->peer->_peerF28 >> 1) & 0b1111u) == 3) {
                        FD_CLR(v19, rfdp);
                        FD_CLR(v19, wfdp);
                    }
                    prevPeer = v10;
                    v10 = v10->next;
                }
            } while (v10);
        }
        if ( _peerlistF9e & 4 )
        {
            _peerlistF9e = 4 * BandWidthLimitUp(this->p_this->p_structure->Self->p_rate_ul->m_late) | (_peerlistF9e & 0b11111011);
            if ( !(_peerlistF9e & 4) ) {
                continue;
            }
            if ( !(_peerlistF9e & 2) )
                break;
        } else {
            if ( !(_peerlistF9e & 2) )
                break;//goto LABEL_32;
        }
        _peerlistF9e = 2 * BandWidthLimitDown(this->p_this->p_structure->Self->p_rate_dl->m_late) | (_peerlistF9e & 0b11111101);
    } while (!(_peerlistF9e & 2));

    if ( !interested_count ) {
        this->p_this->p_structure->Self->p_rate_dl->StopTimer();
    }
    if ( UNCHOKER && bunchoke_check ) {
        this->m_unchoke_check_timestamp = this->p_this->p_structure->now;
        if ( !this->m_opt_timestamp )
            this->m_opt_timestamp = this->p_this->p_structure->now;
        print_dbg(goalbitp2p,
                  "[Unchoking Policy] seeder unchokes: %d, leecher unchokes: %d, optimistic unchokes: %d",
                  this->i_seeders_unchoke,
                  this->i_leechers_unchoke,
                  this->i_opt_unchoke);
        if ( !this->m_max_unchoke ) {
            this->p_this->p_structure->Self->p_rate_ul->StopTimer();
            delete [] UNCHOKER;//operator delete[](UNCHOKER);
            return maxfd;
        }
        v27 = 0;
        b_unchoke_somebody = 0;
        do
        {
            if ( UNCHOKER[v27] && ((UNCHOKER[v27]->_peerF28 >> 1) & 0xF) != 3 )
            {
                v31 = UNCHOKER[v27]->p_rate_ul->m_count_bytes;
                v32 = UNCHOKER[v27]->p_rate_dl->RateMeasure();
                print_dbg(goalbitp2p,
                          "[Unchoking Policy] Unchoke %d: peer %p (download rate from him: %u Bps, uploaded to him: %u KB)",
                          v27,
                          UNCHOKER[v27],
                          v32,
                          v31 >> 0xA);
                v33 = UNCHOKER[v27]->stream->GetSocket();
                if ( UNCHOKER[v27]->SetLocal(1) < 0 )
                {
                    print_err(goalbitp2p, "close: Can't unchoke peer");
                    UNCHOKER[v27]->CloseConnection();
                    //v47 = _fdelt_warn(v33);
                    //v48 = __ROL4__(0xFFFFFFFE, (char)v33 % 0x20);
                    //rfdp->fds_bits[v47] &= v48;
                    //v49 = _fdelt_warn(v33);
                    //wfdp->fds_bits[v49] &= v48;
                    FD_CLR(v33, rfdp);
                    FD_CLR(v33, wfdp);
                }
                else
                {
//                    v34 = wfdp->fds_bits[_fdelt_warn(v33)];
//                    if ( !_bittest(
//                            &v34,
//                            (((_BYTE)v33 + ((unsigned int)(v33 >> 0x1F) >> 0x1B)) & 0x1F) - ((unsigned int)(v33 >> 0x1F) >> 0x1B))
//                         && (*v29)->NeedWrite((_peerlistF9e >> 2) & 1) )
                    if (!FD_ISSET(v33, wfdp) && UNCHOKER[v27]->NeedWrite((_peerlistF9e >> 2) & 1) )
                    {
                        //v46 = _fdelt_warn(v33);
                        //wfdp->fds_bits[v46] |= 1 << (char)v33 % 0x20;
                        FD_SET(v33, wfdp);
                        if ( maxfd < v33 )
                            maxfd = v33;
                    }
                    b_unchoke_somebody = 1;
                }
            }
        } while ( ++v27 < this->m_max_unchoke );
        if ( !b_unchoke_somebody ) {
            this->p_this->p_structure->Self->p_rate_ul->StopTimer();
        }
        delete [] UNCHOKER;//operator delete[](UNCHOKER);
    }
    return maxfd;
}

int PeerList::UnChokeCheck(Peer *peer, Peer **peer_array) {
    Peer *v3; // edx@2
    signed int v4; // edi@4
    signed int v5; // esi@4
    Peer** v6; // ebx@4
    Peer** v7; // ebp@5
    Peer** v8; // ebp@8
    size_t v9; // ST18_4@8
    size_t v10; // ST18_4@10
    int result; // eax@15
    Peer* v12; // eax@17
    Rate* v13; // ecx@17
    long double v14; // fst7@17
    Rate *v15; // edx@19
    long double v16; // fst7@19
    long double v17; // fst6@19
    Rate* v18; // edx@21
    long double v19; // fst7@21
    long double v20; // fst6@21
    Rate *v21; // eax@23
    long double v22; // fst6@23
    long double v23; // fst5@23
    size_t v24; // ebx@28
    Peer * v25; // esi@29
    Peer ** v26; // edi@33
    int v27; // ebp@35
    Peer **v28; // ebx@35
    Peer *v29; // eax@36
    Peer *v30; // eax@40
    int v31; // edx@48
    Peer *v32; // eax@52
    size_t v33; // ebx@58
    Rate *v34; // eax@59
    long double v35; // fst7@59
    long double v36; // fst7@61
    long double v37; // fst6@61
    Rate *v38; // eax@63
    long double v39; // fst7@63
    long double v40; // fst6@63
    long double v41; // fst6@65
    long double v42; // fst5@65
    size_t i_quality; // [sp+14h] [bp-38h]@1
    size_t i_qualitya; // [sp+14h] [bp-38h]@42
    int i_curr_opt_unchoke; // [sp+20h] [bp-2Ch]@1

    i_curr_opt_unchoke = this->i_opt_unchoke;
    i_quality = this->i_leechers_unchoke - i_curr_opt_unchoke;
    if ( this->i_leechers_unchoke == i_curr_opt_unchoke || (v3 = *peer_array) == 0 )
    {
        v7 = peer_array;
        goto LABEL_15;
    }
    if ( ((v3->_peerF28 >> 1) & 0xF) == 3 )
    {
        v7 = peer_array;
    }
    else
    {
        v4 = 0;
        v5 = 1;
        v6 = peer_array + 1;
        if ( 1 != i_quality )
        {
            do
            {
                v3 = *v6;
                v7 = v6;
                if ( !*v6 )
                    goto LABEL_15;
                if ( ((v3->_peerF28 >> 1) & 0xF) == 3 )
                    goto LABEL_13;
                if ( v4 != v5 )
                {
                    v8 = &peer_array[v4];
                    v9 = v8[0]->p_rate_dl->RateMeasure();
                    if ( v9 <= (*v6)->p_rate_dl->RateMeasure() )
                    {
                        v10 = v8[0]->p_rate_dl->RateMeasure();
                        if ( v10 == v6[0]->p_rate_dl->RateMeasure() )
                        {
                            v12 = *v8;
                            v13 = (*v6)->p_rate_ul;
                            v14 = v13->m_count_bytes;
                            if ( v14 < 0 )
                                v14 = v14 + 1.8446744e19;
                            v15 = (*v6)->p_rate_dl;
                            v16 = (double)v14;
                            v17 = (long double)v15->m_count_bytes;
                            if ( v15->m_count_bytes < 0 )
                                v17 = v17 + 1.8446744e19;
                            v18 = v12->p_rate_ul;
                            v19 = v16 / ((double)v17 + 0.001);
                            v20 = v18->m_count_bytes;
                            if ( v18->m_count_bytes < 0 )
                                v20 = v20 + 1.8446744e19;
                            v21 = v12->p_rate_ul;
                            v22 = (double)v20;
                            v23 = (long double)v21->m_count_bytes;
                            if ( v21->m_count_bytes < 0 )
                                v23 = v23 + 1.8446744e19;
                            if ( v19 > v22 / ((double)v23 + 0.001) )
                                v4 = v5;
                        }
                    }
                    else
                    {
                        v4 = v5;
                    }
                }
                ++v6;
            }
            while ( ++v5 != i_quality );
        }
        v7 = &peer_array[v4];
        v3 = *v7;
    }
    LABEL_13:
    if ( !v3 || ((v3->_peerF28 >> 1) & 0xF) != 2 )
    {
        LABEL_15:
        result = 0;
        *v7 = peer;
        return result;
    }
    v24 = peer->p_rate_dl->RateMeasure();
    if ( v24 <= (*v7)->p_rate_dl->RateMeasure() )
    {
        v33 = (*v7)->p_rate_dl->RateMeasure();
        v25 = peer;
        if ( v33 != peer->p_rate_dl->RateMeasure() )
            goto LABEL_31;
        v25 = *v7;
        v34 = (*v7)->p_rate_ul;
        v35 = (long double)v34->m_count_bytes;
        if ( v34->m_count_bytes < 0 )
            v35 = v35 + 1.8446744e19;
        v36 = (double)v35;
        v37 = (long double)v25->p_rate_dl->m_count_bytes;
        if ( v25->p_rate_dl->m_count_bytes < 0 )
            v37 = v37 + 1.8446744e19;
        v38 = peer->p_rate_ul;
        v39 = v36 / ((double)v37 + 0.001);
        v40 = (long double)v38->m_count_bytes;
        if ( v38->m_count_bytes < 0 )
            v40 = v40 + 1.8446744e19;
        v41 = (double)v40;
        v42 = (long double)peer->p_rate_dl->m_count_bytes;
        if ( peer->p_rate_dl->m_count_bytes < 0 )
            v42 = v42 + 1.8446744e19;
        if ( v39 <= v41 / ((double)v42 + 0.001) )
        {
            v25 = peer;
            goto LABEL_31;
        }
    }
    else
    {
        v25 = *v7;
    }
    *v7 = peer;
    LABEL_31:
    if ( i_curr_opt_unchoke <= 0 )
        goto LABEL_72;
    if ( this->i_leechers_unchoke > i_quality )
    {
        v26 = &peer_array[i_quality];
        if ( !*v26 || ((v26[0]->_peerF28 >> 1) & 0xF) == 3 )
        {
            LABEL_54:
            v26[0] = v25;
            return 0;
        }
        v27 = this->i_leechers_unchoke - i_curr_opt_unchoke;
        v28 = &peer_array[i_quality + 1];
        while ( 1 )
        {
            i_qualitya = this->p_this->p_structure->i_target_quality;
            if ( v25->IsEmpty(this->p_this->p_structure->i_target_quality)
                 && !(*v26)->IsEmpty(i_qualitya)
                 && (long double)random() / 2147483647.0 <= 0.75 )
            {
                LABEL_52:
                v32 = (Peer *)*v26;
                *v26 = v25;
                v25 = v32;
                goto LABEL_39;
            }
            if ( v25->m_state & 4 )
            {
                v29 = *v26;
                if ( (*v26)->m_state & 4 && v25->m_unchoke_timestamp >= v29->m_unchoke_timestamp )
                {
                    if ( this->i_leechers_unchoke <= ++v27 )
                        break;
                    goto LABEL_40;
                }
            }
            else
            {
                v29 = *v26;
                if ( (*v26)->m_state & 4 || v29->m_unchoke_timestamp >= v25->m_unchoke_timestamp )
                    goto LABEL_39;
            }
            if ( v29->IsEmpty(i_qualitya)
                 && !v25->IsEmpty(i_qualitya)
                 && (long double)random() / 2147483647.0 <= 0.25 )
            {
                goto LABEL_52;
            }
            LABEL_39:
            if ( this->i_leechers_unchoke <= ++v27 )
                break;
            LABEL_40:
            v30 = *v28;
            v26 = v28;
            if ( *v28 )
            {
                ++v28;
                if ( ((v30->_peerF28 >> 1) & 0xF) != 3 )
                    continue;
            }
            goto LABEL_54;
        }
    }
    if ( v25 )
    {
        LABEL_72:
        v31 = v25->SetLocal(0);
        result = 0;
        if ( v31 < 0 )
        {
            v25->CloseConnection();
            result = (peer != v25) - 1;
        }
    }
    else
    {
        result = 0;
    }
    return result;
}


#ifdef WIN32

int setfd_nonblock(SOCKET socket)
{
    unsigned long val = 1;
    return ioctl(socket,FIONBIO,&val);
}

#else

#include <unistd.h>
#include <fcntl.h>

int setfd_nonblock(SOCKET socket)
{
    int f_old = fcntl(socket, F_GETFL, 0);
    if( f_old < 0 ) return -1;
    f_old |= O_NONBLOCK;
    return fcntl(socket, F_SETFL, f_old);
}

#endif

int connect_nonb(SOCKET sk, struct sockaddr* psa)
{
    int r = connect(sk, psa, sizeof(struct sockaddr));
    int err = errno;
    if(r < 0 && err == EINPROGRESS) r = -2;
#ifdef WIN32
    if(r < 0 && WSAGetLastError() == WSAEWOULDBLOCK) r = -2;
#endif
    if ( r < 0 && ( err == EINPROGRESS || err == EWOULDBLOCK ) ) r = -2;

    return r;
}

unsigned int PeerList::NewPeer(sockaddr_in addr, SOCKET sk, unsigned int score) {

    BT_PEERNODE *v11; // ebx@6
    BT_PEERNODE *v12; // eax@7
    Peer *v13; // edi@11
    BT_PEERNODE *v14; // ebp@11
    BT_PEERNODE *v20; // eax@20
    unsigned int v22; // esi@21
    int v28; // edi@29
    char msg_0[255]; // [sp+3Dh] [bp-21Fh]@31
    sockaddr_in bindaddr; // [sp+13Ch] [bp-120h]@17

    for ( BT_PEERNODE *i = this->m_head; i!=0&&((i->peer->_peerF28 >> 1) & 0xF) != 3; i = i->next ) {
        if ( i->peer->AddrEquiv(addr) ) {// remove duplicated socket
            if ( sk != -1 ) {
                print_wrn(goalbitp2p, "Connection from duplicate peer %s", inet_ntoa(addr.sin_addr));
                shutdown(sk, 2);
                close(sk);
            }
            return -3;
        }
    }

    print_wrn(goalbitp2p, "Connection to/from %s:%u", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    BT_PEERNODE *v9 = this->m_dead;
    if ( v9 ) {// remove dead element expired after a certain time
        //goto LABEL_14;
        v11 = 0;
        do {
            v13 = v9->peer;
            v14 = v9->next;
            if (v9->peer->m_last_timestamp+2*p_this->p_structure->Tracker->m_default_interval < p_this->p_structure->now) {
                delete v13;
                v12 = v9->next;
                if (v11)
                    v11->next = v12;
                else
                    m_dead = v12;
                delete v9;//operator delete(v9);
            } else {
                v11 = v9;
            }
            v9 = v14;
        } while (v9);
    }
    if (sk == -1) {
        sk = socket(AF_INET, SOCK_STREAM, 0);
        if (sk == -1) {
            return -1;
        }
        if (setfd_nonblock(sk) < 0) {
            shutdown(sk, -1);
            close(sk);
            return -1;
        }
        if (p_this->p_param->l_listen_ip) {
            memset(&bindaddr, 0, sizeof(bindaddr));
            bindaddr.sin_family = 2;
            bindaddr.sin_addr.s_addr = p_this->p_param->l_listen_ip;
            if (bind(sk, (sockaddr *) &bindaddr, sizeof(sockaddr))) {
                print_wrn(goalbitp2p, "Warn, cannot bind outgoing connection:  %s", strerror(errno));
            }
        }
        v28 = connect_nonb(sk, (sockaddr *) &addr);
        if (v28 == -1) {
            print_wrn(goalbitp2p, "Connect to peer at %s:%hu failed:  %s", inet_ntoa(addr.sin_addr),
                      ntohs(addr.sin_port), strerror(errno));
            return -1;
        }
    } else {
        v28 = 0;
    }
    if (setfd_nonblock(sk) < 0) {
        shutdown(sk, -1);
        close(sk);
        return -1;
    }
    Peer* v15 = new Peer(p_this);
    if ( !v15 ) {
        shutdown(sk, -1);
        close(sk);
        return -1;
    }
    v15->SetAddress(addr);
    v15->stream->SetSocket(sk);
    v15->_peerF28 |= 0b10000000;
    v15->_peerF28 &= 0b1110001;
    if (v28 != -2) {
        v15->_peerF28 != 0b10;
    }
    v15->score = score;
    sprintf(msg_0, "Connecting to %s:%hu (peer %p)", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), v15);
    print_wrn(goalbitp2p, msg_0);
    if ( !v15->stream->GetInBufferLeftSize() && !v15->stream->CanIncreaseInBuffer()) {
        delete v15;
        shutdown(sk, -1);
        close(sk);
        return -1;
    }
    if ((v15->_peerF28 >> 1) & 0x0F == 1) {
        if (v15->Send_ShakeInfo()) {
            delete v15;
            shutdown(sk, -1);
            close(sk);
            return -1;
        }
    }
    v20 = new BT_PEERNODE();
    if (v20 == 0) {
        delete v15;
        shutdown(sk, -1);
        close(sk);
        return -1;
    }
    ++m_peers_count;
    v20->peer = v15;
    v20->next = m_head;
    m_head = v20;
    return 0;
}

void PeerList::UnchokeIfFree(Peer *pPeer) {
    BT_PEERNODE *v2; // edx@1
    int v3; // esi@2
    PEER_STATUS v4; // al@5

    v2 = this->m_head;
    if ( v2 )
    {
        v3 = 0;
        do {
            if (((v2->peer->_peerF28 >> 1) & 0xF) == 2) {
                if (!(v2->peer->m_state & 4) && (v2->peer->m_state & 2)) {
                    if (++v3 > this->m_max_unchoke)
                        return;
                }
            }
            v2 = v2->next;
        } while ( v2 );
    }
    if ( pPeer->SetLocal(1) < 0 )
        pPeer->CloseConnection();
}

int __ROL4__(unsigned int i, int i1) {
    for (int i = 0; i < i1; ++i) {
        i = i<<2;
    }
    return i;
}

//bool _bittest(void *pInt, unsigned int v45) {
//    return false;
//}

TrackerServer::TrackerServer(goalbit_t_0 *p_goalbit, btTracker *p_tracker) {
    this->p_this = p_goalbit;
    this->p_parent = p_tracker;
    this->p_events_queue = Init_Queue();
}

void TrackerServer::Run() {
    queue_node_t *v1; // eax@5
    //queue_node_t *v3; // ST1C_4@11
    queue_node_t *v4; // eax@9

    print_dbg(__func__, "[TrackerServer] Running main loop");
    // if final two bits are all set exit
    while ( (this->p_parent->_trackerF46 & 0b11u) != 0b11u )
    {
        v1 = Pop_Value(this->p_events_queue);
        if ( v1 )
        {
            if ( v1->event.i_event == 1 )
            {
                sendHTTPrequestToTracker(this->p_this, v1->event.value.psz_string);
                free(v1->event.value.psz_string);
            }
            free(v1);
        }
        msleep(50000LL);
    }
    while ( v4 = Pop_Value(this->p_events_queue) ) {
        if ( v4->event.i_event == 1 )
        {
            //v3 = v4;
            free(v4->event.value.psz_string);
            //v4 = v3;
        }
        free(v4);
    }

}

void TrackerServer::sendHTTPrequestToTracker(goalbit_t_0 *p_this, char *psz_request_url) {

    print_dbg(__func__, "[TrackerServer] sendHTTPrequestToTracker - Incomming request: %s", psz_request_url);
    block_t *p_response = sendHTTPRequest(p_this, psz_request_url, 0x40000, 0xA);
    if ( p_response )
    {
        print_dbg(__func__, "[TrackerServer] sendHTTPrequestToTracker\n\nResponse: %s\n", p_response->p_buffer);
        Mypthread_mutex_lock(92, &this->p_parent->trackercom_lock);
        this->p_parent->setTrackerResponse(p_response);
        Mypthread_mutex_unlock(92, &this->p_parent->trackercom_lock);
        block_Release(p_response);
    }

}

BasicPeer::BasicPeer(goalbit_t_0 *p_goalbit) {

    this->p_this = p_goalbit;
    this->p_rate_dl = new Rate(this->p_this);
    this->p_rate_ul = new Rate(this->p_this);
    this->m_latency = 0;
}

bool BasicPeer::AddrEquiv(sockaddr_in addr) {

    if ( !memcmp(&this->m_sin.sin_port, &addr.sin_port, sizeof(this->m_sin.sin_port)) && !memcmp(&this->m_sin.sin_addr, &addr.sin_addr.s_addr, sizeof(this->m_sin.sin_addr)))
    {
        return true;
    }
    return false;
}

void BasicPeer::SetIp(sockaddr_in addr) {
    this->m_sin.sin_addr.s_addr = addr.sin_addr.s_addr;
}

void BasicPeer::SetPort(sockaddr_in addr) {
    this->m_sin.sin_port = addr.sin_port;
}

void BasicPeer::SetAddress(sockaddr_in addr) {
    m_sin = addr;
}

Rate::Rate(goalbit_t_0 *p_goalbit) {

    this->m_prev_realtime = 0.0;
    this->_bf40 &= 0b11111110;
    this->m_recent_realtime = 0.0;
    this->p_this = p_goalbit;
    this->m_last_realtime = 0.0;
    this->m_total_timeused = 0;
    this->m_late = 0.0;
    this->m_last_timestamp = 0;
    this->m_count_bytes = 0LL;
    this->m_prev_size = 0;
    this->m_recent_size = 0;
    this->m_last_size = 0;
    this->m_selfrate = 0;
    int v2 = 0;
    do
    {
        this->p_history[v2].bytes = 0;
        this->p_history[v2].timestamp = 0.0;
        ++v2;
    } while ( v2 != 30 );
    this->i_history_last = 0;
}

size_t Rate::RateMeasure() {
    long double v1; // fst7@1
    int v2; // eax@3
    unsigned int v3; // ecx@3
    long double v4; // fst7@3
    time_t v5; // esi@5
    long double v6; // fst6@8
    unsigned int v7; // esi@10
    unsigned int v8; // edi@11
    long double v9; // fst6@11
    long double v10; // fst6@12
    int v11; // eax@13

    v1 = (long double)(this->p_this->p_structure->now
                       - (signed int)this->p_history[(this->i_history_last + 1) % 30].timestamp);
    if ( v1 == 0.0 )
    {
        v1 = 1.0;
    }
    else
    {
        v11 = 0;
        if ( v1 < 0.0 )
        {
            do
            {
                this->p_history[v11].bytes = 0;
                this->p_history[v11].timestamp = 0.0;
                ++v11;
            } while ( v11 != 30 );
            this->i_history_last = 0;
        }
    }
    Cleanup();
    v2 = 0;
    v3 = 0;
    v4 = (double)v1;
    do
    {
        v3 += this->p_history[v2].bytes;
        ++v2;
    }
    while ( v2 != 30 );
    v5 = this->p_this->p_structure->now;
    if ( v5 < (signed int)this->m_recent_realtime )
    {
        this->m_recent_realtime = (long double)v5;
        this->m_prev_realtime = (long double)(v5 - 1);
        this->m_prev_size = 0;
        this->m_recent_size = 0;
    }
    if ( v3 )
    {
        v6 = this->m_recent_realtime;
        if ( v5 == (signed int)v6 )
        {
            v4 = v6 - this->p_history[(this->i_history_last + 1) % 30].timestamp;
            v10 = (long double)(v3 - this->m_recent_size);
        }
        else if ( v6 == 0.0
                  || (v7 = v5 - (signed int)v6, (signed int)v7 > 0x1D)
                  || (v8 = this->m_recent_size,
                v9 = (long double)this->m_prev_size / (v6 - this->m_prev_realtime),
                (long double)(v8 / v7) <= v9) )
        {
            v10 = (long double)v3;
        }
        else
        {
            v10 = (long double)(v3 + (unsigned int)(__int64)(v9 * (long double)(signed int)v7) - v8);
        }
    }
    else
    {
        v10 = 0.0;
    }
    return (__int64)(v10 / v4);
}

void Rate::Reset() {
    this->m_last_realtime = 0;
    this->m_total_timeused = 0;
    this->m_last_timestamp = 0;
    this->m_last_size = 0;
    int i = 0;
    do {
        this->p_history[i].bytes = 0;
        this->p_history[i].timestamp = 0;
    } while (++i != 30);
    this->i_history_last = 0;
}

void Rate::Cleanup() {
    int v1; // edi@1
    int v2; // edx@1
    long double v3; // fst7@1
    int v4; // ebx@4
    int v5; // ebp@4
    int v6; // eax@6
    int v7; // ecx@6
    //char *v8; // eax@6

    v1 = 30;
    v2 = this->i_history_last;
    v3 = this->p_history[v2].timestamp;
    if ( this->p_this->p_structure->now - this->p_history[v2].timestamp <= 30 )
        v1 = this->p_this->p_structure->now - (signed int)v3;
    if ( v1 > 0 )
    {
        v4 = 1;
        v5 = this->p_this->p_structure->now - v1;
        do
        {
            v2 = (v2 + 1) % 30;
            this->p_history[v2].timestamp = v5 + v4 + 1;//*(double *)(v8 + 0x44) = (long double)v7;
            this->p_history[v2].bytes = 0;//*((_DWORD *)v8 + 0x13) = 0;
            v4++;
        } while ( v1 >= v4 );
        this->i_history_last = v2;
    }
}

void Rate::RateAdd(size_t nbytes, size_t bwlimit, double timestamp) {
    Rate *i; // esi@1
    int v6; // eax@2
    int v7; // ebp@5
    int v8; // ebx@5
    //int v9; // eax@7
    _bwsample* v10; // ecx@7
    int v11; // eax@8
    Rate *v12; // eax@11
    size_t v13; // edx@17
    char v14; // cl@23
    bool v15; // dl@23
    long double v16; // fst6@24
    long double v17; // fst5@25
    long double v18; // t0@26
    size_t v19; // edx@32
    long double v20; // fst5@36
    long double v21; // fst6@36

    for ( i = this; ; i = v12 )
    {
        v6 = 0;
        if ( i->p_history[i->i_history_last].timestamp > timestamp )
        {
            do
            {
                i->p_history[v6].bytes = 0;
                i->p_history[v6].timestamp = 0.0;
                ++v6;
            }
            while ( v6 != sizeof(i->p_history)/ sizeof(_bwsample) );
            i->i_history_last = 0;
        }
        i->Cleanup();
        //v4 = (double)v4;
        if ( i->m_last_realtime >= timestamp )
        {
            i->_bf40 &= 0b11111110;
            i->m_last_realtime = 0.0;
            i->m_last_size = 0;
        }
        v7 = i->i_history_last;
        v8 = 0;
        while ( 1 )
        {
            //v9 = i + 0xC * ((v7 - v8) % 0x1E);
            v10 = &i->p_history[v7-v8];//v10 = v9 + 0x40;
            if ( v10->timestamp == timestamp )
                break;
            if ( ++v8 == 0x1E )
                goto LABEL_11;
        }
        v11 = v10->bytes;
        //        if ( !v11 )
        //            *(double *)(v10 + 4) = v4;
        i->p_history[(v7 - v8) % 0x1E].bytes = nbytes + v11;
        LABEL_11:
        v12 = i->m_selfrate;
        if ( v12 )
        {
            if ( !bwlimit )
                goto LABEL_29;
            if ( i->m_last_realtime != 0.0
                 && (long double)v12->m_last_size / (timestamp - v12->m_last_realtime) > (long double)bwlimit )
            {
                i->m_last_size += nbytes;
                goto LABEL_16;
            }
        }
        else
        {
            v14 = i->_bf40;
            v15 = bwlimit != 0;
            if ( v14 & 1 )
            {
                v16 = timestamp - ((long double)i->m_last_size / (long double)bwlimit + i->m_last_realtime);
                if ( v16 >= 0.0 )
                {
                    v17 = i->m_late;
                    if ( v17 != 0.0 )
                    {
                        v20 = v16;
                        v21 = i->m_late;
                        if ( v20 <= v21 )
                        {
                            v18 = v20;
                            v17 = i->m_late;
                            v16 = v18;
                        }
                        else
                        {
                            v17 = i->m_late;
                            v16 = v21 * 0.5;
                        }
                    }
                }
                else
                {
                    v16 = v16 * 0.5;
                    v17 = i->m_late;
                }
                i->_bf40 = v14 & 0b11111110;
                i->m_late = v16 + v17;
                v15 = bwlimit != 0;
            }
            if ( !v15 )
            {
                LABEL_29:
                i->m_last_realtime = timestamp;
                i->m_last_size = nbytes;
                goto LABEL_16;
            }
        }
        if ( v12 )
            goto LABEL_29;
        if ( i->m_last_realtime == 0.0 )
            goto LABEL_29;
        v19 = i->m_last_size;
        if ( (long double)v19 / (timestamp - i->m_last_realtime) <= (long double)bwlimit )
            goto LABEL_29;
        i->m_last_size = nbytes + v19;
        LABEL_16:
        if ( nbytes > 0xFA000 )
            break;
        i->m_recent_size += nbytes;
        if ( !v12 )
            return;
        LABEL_18:
        ;
    }
    v13 = i->m_recent_size;
    i->m_prev_realtime = i->m_recent_realtime;
    i->m_recent_realtime = timestamp;
    i->m_prev_size = v13;
    i->m_recent_size = nbytes;
    if ( v12 )
        goto LABEL_18;
}

void Rate::StartTimer() {
    if ( !this->m_last_timestamp )
        this->m_last_timestamp = this->p_this->p_structure->now;
}

void Rate::StopTimer()
{
    if ( this->m_last_timestamp )
    {
        this->m_last_timestamp = 0;
        this->m_total_timeused += this->p_this->p_structure->now - this->m_last_timestamp;
    }
}

int Rate::CurrentRate() {
    int v1; // esi@1
    long double v3; // fst7@4
    timespec timestamp; // [sp+28h] [bp-14h]@2

    v1 = 0;
    if ( this->m_last_timestamp )
    {
        clock_gettime(_CLOCK_REALTIME, &timestamp);
        v3 = (long double)timestamp.tv_sec + (long double)timestamp.tv_nsec / 1000000000.0 - this->m_last_realtime;
        if ( v3 > 0.0 )
            v1 = ((long double)this->m_last_size / v3);
    }
    return v1;
}

void Rate::CountAdd(size_t nbytes) {
    Rate *v2; // eax@1

    v2 = this;
    do
    {
        v2->m_count_bytes += nbytes;
        v2 = v2->m_selfrate;
    }
    while ( v2 );
}

PeerPendingQueue::PeerPendingQueue(goalbit_t_0 *p_goalbit) {
    int v2; // eax@1

    this->p_this = p_goalbit;
    v2 = 0;
    do {
        this->pending_array[v2++] = 0;
    } while ( v2 != sizeof(pending_array)/ sizeof(PIECE*) );
    this->i_count = 0;
    return;
}

int PeerPendingQueue::AddPieces(PeerRequestQueue *p_requests) {
    signed int result; // eax@1
    size_t v3; // esi@5
    size_t v4; // ebx@5
    int v5; // eax@6
    int v6; // ST1C_4@7
    PIECE *v7; // eax@7
    PIECE *v8; // ST1C_4@7
    size_t v9; // eax@7
    PIECE *v10; // eax@9

    result = 0;
    if ( p_requests )
    {
        if ( this->i_count <= 0x63 )
        {
            while ( 1 )
            {
                v10 = p_requests->p_head;
                if ( !v10 )
                    break;
                v3 = v10->i_index;
                v4 = v10->i_quality;
                if ( !ExistPiece(v10->i_quality, v10->i_index) )
                {
                    v5 = getFirstEmptyIndex();
                    if ( v5 >= 0 )
                    {
                        v6 = v5;
                        v7 = new PIECE();
                        this->pending_array[v6] = v7;
                        v7->i_index = v3;
                        v7->i_quality = v4;
                        v8 = v7;
                        v9 = p_requests->GetRequestLength(v4, v3);
                        v8->t_request_time = 0;
                        v8->p_next = 0;
                        v8->i_length = v9;
                        ++this->i_count;
                    }
                }
                p_requests->Remove(v4, v3);
            }
            result = 1;
        }
        else
        {
            p_requests->Clear();
            result = 0;
        }
    }
    return result;
}

int PeerPendingQueue::ExistPiece(size_t i_piece_quality, size_t i_piece_id) {
    signed int result; // eax@2
    int v4; // eax@3
    PIECE *v5; // edx@4

    if ( this->i_count )
    {
        v4 = 0;
        while ( 1 )
        {
            v5 = this->pending_array[v4];
            if ( v5 )
            {
                if ( v5->i_index == i_piece_id && v5->i_quality == i_piece_quality )
                    break;
            }
            if ( ++v4 == 0x64 )
                goto LABEL_2;
        }
        result = 1;
    }
    else
    {
        LABEL_2:
        result = 0;
    }
    return result;
}

int PeerPendingQueue::getFirstEmptyIndex() {
    int result; // eax@1
    size_t v2; // ecx@1

    result = -1;
    v2 = this->i_count;
    if ( v2 <= 0x63 )
    {
        result = 0;
        if ( v2 )
        {
            while ( this->pending_array[result] )
            {
                if ( ++result == 0x64 )
                    return -1;
            }
        }
    }
    return result;
}

int PeerPendingQueue::GetPieces(PeerRequestQueue *p_requests, btBitField *p_bitfield) {
    signed int result; // eax@1
    int v4; // edi@3
    PIECE *v5; // eax@4
    size_t v6; // ebx@5
    size_t v7; // esi@5
    PIECE *v8; // eax@6

    result = 0;
    if ( p_bitfield )
    {
        if ( p_requests )
        {
            v4 = 0;
            if ( this->i_count )
            {
                while ( 1 )
                {
                    v5 = this->pending_array[v4];
                    if ( v5 )
                    {
                        v6 = v5->i_quality;
                        v7 = v5->i_index;
                        if ( p_bitfield->IsSet(v5->i_quality, v5->i_index) )
                            break;
                    }
                    LABEL_12:
                    if ( ++v4 == sizeof(pending_array)/ sizeof(PIECE*) || !this->i_count )
                        return 0;
                }
                v8 = p_requests->p_head;
                if ( !v8 )
                {
                    LABEL_19:
                    p_requests->Add(v6, v7, this->pending_array[v4]->i_length, 0);
                    DeletePieceByIndex(v4);
                    return 1;
                }
                while ( 1 )
                {
                    if ( v6 == 0x80000001 )
                    {
                        if ( v7 == v8->i_index )
                            goto LABEL_12;
                        LABEL_9:
                        v8 = v8->p_next;
                        if ( !v8 )
                            goto LABEL_19;
                    }
                    else
                    {
                        if ( v7 != v8->i_index )
                            goto LABEL_9;
                        if ( v6 == v8->i_quality )
                            goto LABEL_12;
                        v8 = v8->p_next;
                        if ( !v8 )
                            goto LABEL_19;
                    }
                }
            }
        }
    }
    return result;
}

int PeerPendingQueue::DeletePieceByIndex(int i_index) {
    signed int result; // eax@1
    PIECE *v3; // edx@2

    result = 0;
    if ( (unsigned int)i_index <= 0x64 )
    {
        v3 = this->pending_array[i_index];
        if ( v3 )
        {
            print_dbg(__func__, "[PENDING QUEUE] delete piece %d (at index %d)", v3->i_index, i_index);
            delete this->pending_array[i_index];//operator delete(this->pending_array[i_index]);
            result = 1;
            --this->i_count;
            this->pending_array[i_index] = 0;
        }
    }
    return result;
}

int PeerPendingQueue::AddPiece(size_t i_piece_quality, size_t i_piece_id) {
    signed int v3; // esi@1
    int v5; // edi@4
    PIECE *v6; // eax@5
    PIECE *v7; // eax@5

    v3 = 0;
    if ( this->i_count <= 0x63 )
    {
        v3 = 1;
        if ( !(unsigned __int8)ExistPiece(i_piece_quality, i_piece_id) )
        {
            v5 = getFirstEmptyIndex();
            if ( v5 >= 0 )
            {
                v6 = new PIECE();
                this->pending_array[v5] = v6;
                v6->i_quality = i_piece_quality;
                v6->i_index = i_piece_id;
                v6->i_length = this->p_this->p_structure->BTCONTENT->GetPieceSize(i_piece_quality, i_piece_id);
                v7 = this->pending_array[v5];
                v7->t_request_time = 0;
                v7->p_next = 0;
                ++this->i_count;
            }
        }
    }
    return v3;
}

int PeerPendingQueue::DeletePieceByID(size_t i_piece_id) {
    int result; // eax@2
    int v3; // ebx@3
    PIECE *v4; // eax@4

    if ( this->i_count )
    {
        v3 = 0;
        while ( 1 )
        {
            v4 = this->pending_array[v3];
            if ( v4 )
            {
                if ( v4->i_index == i_piece_id )
                    break;
            }
            if ( ++v3 == 0x64 )
                goto LABEL_2;
        }
        print_dbg(__func__, "[PENDING QUEUE] delete piece %u", i_piece_id);
        delete(this->pending_array[v3]);
        result = 1;
        this->pending_array[v3] = 0;
        --this->i_count;
    }
    else
    {
        LABEL_2:
        result = 0;
    }
    return result;
}

unsigned int IpList::Add(sockaddr_in *psin, unsigned int i_peer_score) {
    IPLIST *v3; // ebx@1
    unsigned int result; // eax@5
    IPLIST *v5; // eax@6

    v3 = this->ipl_head;
    if ( !v3 )
        goto LABEL_6;
    while ( memcmp(psin, v3, 0x10u) )
    {
        v3 = v3->next;
        if ( !v3 )
            goto LABEL_6;
    }
    result = -1;
    if ( !v3 )
    {
        LABEL_6:
        v5 = new IPLIST();
        if ( v5 )
        {
            ++this->count;
            v5->address = *psin;
            v5->next = this->ipl_head;
            v5->score = i_peer_score;
            this->ipl_head = v5;
            result = 0;
        }
        else
        {
            result = -1;
        }
    }
    return result;
}

metadata_content_t *
MetadataParser::parseMetadata(char *p_buffer, size_t i_length, const char *psz_url) {
    if ( p_buffer && i_length && *p_buffer ) {
        char* p_begin = p_buffer;
        char *v4 = ReadLine(p_buffer, &p_begin, i_length);
        if (!v4) {
            print_err(__func__, "[MetadataParser] no metadata to read");
            return 0;
        }
        if ( strncmp(v4, "#PROGRAM:", sizeof("#PROGRAM:")-1) == 0 ) {
            int i_program_id = 0;
            int i_qualities = 0;
            sscanf(v4, "#PROGRAM:ID=%d,QUALITIES=%d", &i_program_id, &i_qualities);
            free(v4);
            metadata_content_t *v10 = newContent(psz_url, i_program_id);
            if (!v10) {
                print_err(__func__, "[MetadataParser] could not create content");
                return 0;
            }
            if ( i_qualities <= 0 ) {
                return 0;
            }
            do {
                metadata_stream_t *v11 = parseStream(&p_begin, &p_buffer[i_length]);
                if ( !v11 ) {
                    freeContent(v10);
                    return 0;
                }
                array_append(v10->p_streams, v11);
                i_qualities--;
            } while (i_qualities > 0);
            return v10;
        }
        print_err(__func__, "[MetadataParser] invalid start of metadata");
        free(v4);
    }
    return 0;
}

metadata_stream_t *MetadataParser::parseStream(char **p_begin, char *p_end) {
    char *v3; // eax@1
    bool v4; // zf@1
    char *v8; // ST28_4@6
    uint64_t v9; // rax@6
    metadata_stream_t *v10; // esi@6
    metadata_segment_t *v11; // eax@8
    //int v12; // edx@11
    char *v14; // ST28_4@12
    int i_video_width; // [sp+38h] [bp-F4h]@6
    int i_video_height; // [sp+3Ch] [bp-F0h]@6
    int i_segments; // [sp+40h] [bp-ECh]@6
    char psz_name[100]; // [sp+44h] [bp-E8h]@6
    char psz_bitrate[100]; // [sp+A8h] [bp-84h]@6
    //int v20; // [sp+10Ch] [bp-20h]@1

    //v20 = *MK_FP(__GS__, 0x14);
    v3 = ReadLine(*p_begin, p_begin, p_end - *p_begin);
    if ( v3 )
    {
        v4 = strncmp(v3, "#QUALITY:", sizeof("#QUALITY:")-1);
        if ( !v4 )
        {
            v8 = v3;
            i_video_width = 0;
            i_video_height = 0;
            i_segments = 0;
            sscanf(
                    v3,
                    "#QUALITY:NAME=%[^,],BITRATE=%[^,],WIDTH=%d,HEIGHT=%d,SEGMENTS=%d",
                    psz_name,
                    psz_bitrate,
                    &i_video_width,
                    &i_video_height,
                    &i_segments);
            free(v8);
            v9 = strtol(psz_bitrate, 0, 0xA);
            v10 = newStream(psz_name, v9, i_video_width, i_video_height);
            if ( v10 )
            {
                for ( ; i_segments > 0; --i_segments )
                {
                    v11 = parseSegment(p_begin, p_end);
                    if ( v11 )
                        array_append(v10->p_segments, v11);
                }
            }
            else
            {
                print_err(__func__, "[MetadataParser] could not create stream");
            }
        }
        else
        {
            v10 = 0;
            v14 = v3;
            print_err(__func__, "[MetadataParser] invalid start of quality");
            free(v14);
        }
    }
    else
    {
        v10 = 0;
        print_err(__func__, "[MetadataParser] no quality to read");
    }
    //v12 = *MK_FP(__GS__, 0x14) ^ v20;
    return v10;
}

metadata_segment_t * newSegment(const int id, const float duration, const uint64_t size, const char *md5, const bool discontinuity)
{
    metadata_segment_t *v5; // ebx@1
    metadata_segment_t *v6; // eax@3

    v5 = 0;
    if ( md5 )
    {
        if ( *md5 )
        {
            v6 = new metadata_segment_t();
            v5 = v6;
            if ( v6 )
            {
                v6->b_discontinuity = discontinuity;
                v6->i_size = size;
                v6->i_sequence = id;
                v6->i_duration = duration;
                v6->psz_MD5 = (char *)strdup(md5);
                v5->p_pieces = array_new<metadata_piece_t>();
                v5->p_keyframes = array_new<metadata_keyframe_t>();
            }
        }
    }
    return v5;
}

metadata_segment_t *MetadataParser::parseSegment(char **p_begin, char *p_end) {
    char **v3; // ebx@1
    char *v4; // ebp@1
    char *v5; // eax@1
    bool v6; // zf@1
    const char *v7; // edi@2
    signed int v8; // ecx@2
    char *v9; // esi@2
    char *v10; // ST34_4@6
    uint64_t v11; // rax@6
    metadata_segment_t *v12; // eax@6
    metadata_segment_t *v13; // edi@6
    metadata_piece_t *v14; // eax@10
    //int v15; // edx@12
    metadata_keyframe_t *v17; // eax@17
    metadata_segment_t *id; // ST00_4@18
    char *v19; // ST34_4@19
    metadata_segment_t *p_segment; // [sp+38h] [bp-114h]@8
    uint64_t i_piece_offset; // [sp+48h] [bp-104h]@7
    int i_sequence; // [sp+50h] [bp-FCh]@6
    float f_duration; // [sp+54h] [bp-F8h]@6
    int i_pieces; // [sp+58h] [bp-F4h]@6
    int i_keyframes; // [sp+5Ch] [bp-F0h]@6
    int i_discontinuity; // [sp+60h] [bp-ECh]@6
    char psz_MD5[100]; // [sp+64h] [bp-E8h]@6
    char psz_size[100]; // [sp+C8h] [bp-84h]@6
    //int v29; // [sp+12Ch] [bp-20h]@1

    v3 = p_begin;
    v4 = p_end;
    //v29 = *MK_FP(__GS__, 0x14);
    v5 = ReadLine(*p_begin, p_begin, p_end - *p_begin);
    v6 = v5 == 0;
    if ( v5 )
    {
        v7 = "#SEGMENT:";
        v8 = 9;
        v9 = v5;
        v6 = strncmp(v9, "#SEGMENT:", 9);
        if ( !v6 )
        {
            v10 = v5;
            i_sequence = 0;
            f_duration = 0.0;
            i_pieces = 0;
            i_keyframes = 0;
            i_discontinuity = 0;
            psz_MD5[0] = 0;
            psz_size[0] = 0;
            sscanf(
                    v5,
                    "#SEGMENT:ID=%d,DURATION=%f,SIZE=%[^,],MD5=%[^,],DISCONTINUITY=%d,PIECES=%d,KEYFRAMES=%d",
                    &i_sequence,
                    &f_duration,
                    psz_size,
                    psz_MD5,
                    &i_discontinuity,
                    &i_pieces,
                    &i_keyframes);
            free(v10);
            v11 = strtol(psz_size, 0, 0xA);
            v12 = newSegment(i_sequence, f_duration, v11, psz_MD5, i_discontinuity > 0);
            v13 = v12;
            if ( v12 )
            {
                i_piece_offset = 0LL;
                if ( i_pieces > 0 )
                {
                    p_segment = v12;
                    do
                    {
                        v14 = parsePiece(p_begin, p_end, &i_piece_offset);
                        if ( !v14 )
                        {
                            v13 = 0;
                            freeSegment(p_segment);
                            goto LABEL_12;
                        }
                        array_append(p_segment->p_pieces, v14);
                        --i_pieces;
                    } while ( i_pieces > 0 );
                    v3 = p_begin;
                    v4 = p_end;
                    v13 = p_segment;
                }
                if ( i_keyframes > 0 )
                {
                    while ( 1 )
                    {
                        v17 = parseKeyframe(v3, v4);
                        if ( !v17 )
                            break;
                        array_append(v13->p_keyframes, v17);
                        if ( --i_keyframes <= 0 )
                            goto LABEL_12;
                    }
                    id = v13;
                    v13 = 0;
                    freeSegment(id);
                }
            }
            else
            {
                print_wrn(__func__,
                          "[MetadataParser] Could not create segment: SEGMENT:ID=%d,DURATION=%f,SIZE=%s,MD5=%s,DISCONTINUITY=%d,PIECES=%d",
                          i_sequence,
                          f_duration,
                          psz_size,
                          psz_MD5,
                          i_discontinuity,
                          i_pieces);
            }
        }
        else
        {
            v13 = 0;
            v19 = v5;
            print_err(__func__, "[MetadataParser] invalid start of segment");
            free(v19);
        }
    }
    else
    {
        v13 = 0;
        print_err(__func__, "[MetadataParser] no segment to read");
    }
    LABEL_12:
    //v15 = *MK_FP(__GS__, 0x14) ^ v29;
    return v13;
}

metadata_piece_t* newPiece(const int id, const float duration, const uint64_t offset, const uint64_t size, const char *md5)
{
    metadata_piece_t *v5; // ebx@1

    v5 = 0;
    if ( md5 )
    {
        if ( *md5 )
        {
            v5 = new metadata_piece_t();
            if ( v5 )
            {
                v5->i_offset = offset;
                v5->i_piece_id = id;
                //HIDWORD(v5->i_size) = HIDWORD(size);
                v5->i_duration = duration;
                //LODWORD(v5->i_size) = size;
                v5->i_size = size;
                v5->psz_MD5 = (char *)strdup(md5);
            }
        }
    }
    return v5;
}

metadata_piece_t *
MetadataParser::parsePiece(char **p_begin, char *p_end, uint64_t *i_piece_offset) {
    char *v8; // eax@1
    bool v9; // zf@1
    char *v10; // ebx@1
    const char *v11; // edi@2
    signed int v12; // ecx@2
    char *v13; // esi@2
    uint64_t v14; // rax@6
    uint64_t v15; // kr00_8@6
    metadata_piece_t *result; // eax@6
    int i_piece_id; // [sp+3Ch] [bp-F0h]@6
    float i_duration; // [sp+40h] [bp-ECh]@6
    char psz_MD5[100]; // [sp+44h] [bp-E8h]@6
    char psz_size[100]; // [sp+A8h] [bp-84h]@6
    //int v21; // [sp+10Ch] [bp-20h]@1
    //int v22; // [sp+11Ch] [bp-10h]@1
    //int v23; // [sp+120h] [bp-Ch]@1
    //int v24; // [sp+124h] [bp-8h]@1
    //int v25; // [sp+128h] [bp-4h]@1

    //v22 = a1;
    //v23 = a4;
    //v24 = a3;
    //v25 = a2;
    //v21 = *MK_FP(__GS__, 0x14);
    v8 = ReadLine(*p_begin, p_begin, p_end - *p_begin);
    v9 = v8 == 0;
    v10 = v8;
    if ( v8 )
    {
        v11 = "#PIECE:";
        v12 = 7;
        v13 = v8;
        v9 = strncmp(v13, "#PIECE:", 7);
        if ( !v9 )
        {
            i_piece_id = 0;
            i_duration = 0.0;
            sscanf(v8, "#PIECE:ID=%d,SIZE=%[^,],DURATION=%f,MD5=%s", &i_piece_id, psz_size, &i_duration, psz_MD5);
            free(v10);
            v14 = strtol(psz_size, 0, 0xA);
            v15 = v14;
            result = newPiece(i_piece_id, i_duration, *i_piece_offset, v14, psz_MD5);
            *i_piece_offset += v15;
            if ( !result )
            {
                print_err(__func__, "[MetadataParser] could not create piece");
                result = 0;
            }
        }
        else
        {
            print_err(__func__, "[MetadataParser] invalid start of piece");
            free(v10);
            result = 0;
        }
    }
    else
    {
        print_err(__func__, "[MetadataParser] no piece to read");
        result = 0;
    }
    return result;
}

metadata_keyframe_t* newKeyframe(const int byte_offset, const float duration)
{
    metadata_keyframe_t *result; // eax@1

    result = new metadata_keyframe_t();
    if ( result )
    {
        result->i_offset = byte_offset;
        result->i_duration = duration;
    }
    return result;
}

metadata_keyframe_t *MetadataParser::parseKeyframe(char **p_begin, char *p_end) {
    char *v3; // eax@1
    bool v4; // zf@1
    char *v5; // ebx@1
    metadata_keyframe_t *result; // eax@6
    int i_offset; // [sp+28h] [bp-24h]@6
    float i_duration; // [sp+2Ch] [bp-20h]@6

    v3 = ReadLine(*p_begin, p_begin, p_end - *p_begin);
    v4 = v3 == 0;
    v5 = v3;
    if ( v3 )
    {
        v4 = strncmp(v3, "#KEYFRAME:", sizeof("#KEYFRAME:")-1);
        if ( !v4 )
        {
            i_offset = 0;
            i_duration = 0.0;
            sscanf(v3, "#KEYFRAME:OFFSET=%d,DURATION=%f", &i_offset, &i_duration);
            free(v5);
            result = newKeyframe(i_offset, i_duration);
            if ( !result )
            {
                print_err(__func__, "[MetadataParser] could not create keyframe");
                result = 0;
            }
        }
        else
        {
            print_err(__func__, "[MetadataParser] invalid start of keyframe");
            free(v5);
            result = 0;
        }
    }
    else
    {
        print_err(__func__, "[MetadataParser] no keyframe to read");
        result = 0;
    }
    return result;
}

MetadataParser::MetadataParser(goalbit_t_0 *pT_0) {
    p_this = pT_0;
}

unsigned int IpList::Pop(sockaddr_in *psin, unsigned int *i_peer_score) {
    IPLIST *v3; // eax@1
    unsigned int result; // eax@2

    v3 = this->ipl_head;
    if ( v3 )
    {
        --this->count;
        this->ipl_head = v3->next;
        *psin = v3->address;
        *i_peer_score = v3->score;
        free(v3);//operator delete(v3);
        result = 0;
    }
    else
    {
        result = -1;
    }
    return result;
}

void IpList::_Emtpy() {
    IPLIST *i; // eax@1

    for ( i = this->ipl_head; i; i = this->ipl_head )
    {
        this->ipl_head = i->next;
        delete i;
    }
    this->count = 0;
}

SOCKET btStream::GetSocket() {
    SOCKET result; // eax@1

    result = this->sock;
    if ( result == -1 )
        result = this->sock_was;
    return result;
}

int btStream::GetBufferedMessageType() {
    if ( HasBufferedMessage() )
    {
        size_t i_length = 5;
        char *v3 = this->buffer_in.Get(&i_length);
        if ( v3 )
        {
            if ( i_length > 4 )
            {
                int v1 = v3[4];
                free(v3);
                return v1;
            }
            else if ( i_length )
            {
                free(v3);
            }
        }
    }
    return 0x30;
}

int get_number(char* a1) {
    int number = (unsigned char)a1[3]+(unsigned char)a1[2]*0x100+(unsigned char)a1[1]*0x10000+(unsigned char)a1[0]*0x1000000;
    return number;
}

bool btStream::HasBufferedMessage() {
    if ( this->buffer_in.i_buffer_count > 3 )
    {
        size_t v5 = 4;
        char* v2 = this->buffer_in.Get(&v5);
        size_t v3 = get_number(v2);
        if ( v2 && v5)
        {
            free(v2);
        }
        return this->buffer_in.i_buffer_count >= v3 + 4;
    }
    return 0;
}

ssize_t btStream::Feed() {
    return this->buffer_in.Feed(this->sock);
}

ssize_t btStream::Feed(Rate *rate) {
    ssize_t v3; // edi@3
    char *v4; // esi@3
    unsigned int v7; // eax@8
    size_t v8; // eax@10
    size_t v9; // ecx@10
    timespec nowspec; // [sp+24h] [bp-18h]@1
    size_t i_length; // [sp+2Ch] [bp-10h]@3

    clock_gettime(_CLOCK_REALTIME, &nowspec);
    if ( nowspec.tv_sec != this->p_this->p_structure->now )
    {
        nowspec.tv_sec = this->p_this->p_structure->now;
        nowspec.tv_nsec = 0;
    }
    v3 = this->buffer_in.Feed(this->sock);
    i_length = -1;
    v4 = this->buffer_in.Get(&i_length);
    if ( v4 )
    {
        if ( i_length > 0x11 )
        {
            if ( v4[4] != 7 )
            {
                free(v4);
                return v3;
            }
            v7 = get_number(v4);
            if ( v7 > 0x11 )
            {
                if ( v7 > i_length )
                {
                    v9 = i_length - 0x11 - this->m_oldbytes;
                    this->m_oldbytes = i_length - 0x11;
                }
                else
                {
                    v8 = v7 - this->m_oldbytes;
                    this->m_oldbytes = 0;
                    v9 = v8 - 0x11;
                }
                rate->RateAdd(
                        v9,
                        this->p_this->p_param->i_max_bandwidth_down,
                        (long double)nowspec.tv_sec + (long double)nowspec.tv_nsec / 1000000000.0);
            }
        }
        if ( i_length ) {
            free(v4);
        }
    }
    return v3;
}

int btStream::GetBufferedMessageLength() {
    if ( HasBufferedMessage() )
    {
        size_t i_length = 4;
        char *v3 = this->buffer_in.Get(&i_length);
        int v1 = get_number(v3);
        if ( v3 )
        {
            if ( i_length )
                free(v3);
        }
        return v1;
    }
    return -1;
}

void set_number(char* a1, size_t a2) {
    a1[0] = a2/0x1000000;
    a1[1] = a2/0x10000;
    a1[2] = a2/0x100;
    a1[3] = a2;
}

ssize_t btStream::Send_State(unsigned __int8 state) {
    char msg[5]; // [sp+17h] [bp-15h]@1

    set_number(msg, 1u);
    msg[4] = state;
    print_wrn(goalbitp2p, "Send_State state %d", state);
    return this->buffer_out.Put(this->sock, msg, 5u);
}

unsigned int btStream::Send_Cancel(size_t i_piece_quality, size_t i_piece_id) {
    unsigned int result; // eax@1
    //int v4; // edx@1
    char msg[13]; // [sp+1Fh] [bp-1Dh]@1
    //int v6; // [sp+2Ch] [bp-10h]@1

    //v6 = *MK_FP(__GS__, 0x14);
    set_number(msg, 9u);
    msg[4] = 8;
    set_number(&msg[5], i_piece_id);
    set_number(&msg[9], i_piece_quality);
    print_wrn(goalbitp2p, "Send_Cancel quality %d piece %d", i_piece_quality, i_piece_id);
    result = this->buffer_out.Put(this->sock, msg, 0x0Du);
    //v4 = *MK_FP(__GS__, 0x14) ^ v6;
    return result;
}

char *btStream::GetBufferedMessage(size_t *const i_length) {
    return this->buffer_in.Get(i_length);
}

ssize_t btStream::Send_Request(size_t i_piece_quality, size_t i_piece_id) {
    ssize_t result; // eax@1
    //int v4; // edx@1
    char msg[13]; // [sp+1Fh] [bp-1Dh]@1
    //int v6; // [sp+2Ch] [bp-10h]@1

    //v6 = *MK_FP(__GS__, 0x14);
    set_number(msg, 9u);
    msg[4] = 6;
    set_number(&msg[5], i_piece_id);
    set_number(&msg[9], i_piece_quality);
    print_wrn(goalbitp2p, "Send_Request quality %d piece %d", i_piece_quality, i_piece_id);
    result = this->buffer_out.PutAndFlush(this->sock, msg, 0x0Du);
    //v4 = *MK_FP(__GS__, 0x14) ^ v6;
    return result;
}

size_t btStream::GetInBufferCount() {
    return this->buffer_in.i_buffer_count;
}

ssize_t btStream::Send_Bitfield(size_t i_win_offset, size_t i_length, char *p_bitfield) {
    char *v4; // esp@1
    ssize_t result; // eax@2
    //int v8; // edx@2
    size_t v10; // [sp+2Ch] [bp-2Ch]@1
    //int v11; // [sp+3Ch] [bp-1Ch]@1

    v4 = (char*)malloc(i_length + 0x18);
    //LODWORD(v5) = this->p_this;
    v10 = i_length + 9;
    print_wrn(goalbitp2p,
              "[btStream] BITFIELD_OUT i_win_offset=%d i_length=%d total_size=%d %lld",
              i_win_offset,
              i_length,
              i_length + 9,
              mdate(),
              this->p_this);
    set_number(v4, i_length + 5);
    *(v4+4) = 5;//*(_BYTE *)((((unsigned int)&v9 + 3) & 0xFFFFFFF0) + 4) = 5;
    set_number(v4 + 5, i_win_offset);//set_number((char *)((((unsigned int)&v9 + 3) & 0xFFFFFFF0) + 5), i_win_offset);
    if ( p_bitfield && i_length )
        memcpy(v4 + 9, p_bitfield, i_length);
    ssize_t s = this->buffer_out.PutAndFlush(this->sock, v4, v10);
    free(v4);
    return s;
}

size_t btStream::RemoveBytes(size_t i_bytes_count) {
    size_t result; // eax@1

    result = 0;
    if ( i_bytes_count )
    {
        this->buffer_in.Remove(&i_bytes_count);
        result = i_bytes_count;
    }
    return result;
}

void btStream::Close() {

    if ( this->sock != -1 )
    {
        print_err(__func__, "CLOSE_SOCKET(%d)", this->sock);
        shutdown(this->sock, 2);
        close(this->sock);
        this->sock = -1;
        this->sock_was = this->sock;
    }
    this->buffer_in.Close();
    this->buffer_out.Close();
}

size_t btStream::GetOutBufferCount() {
    return this->buffer_out.i_buffer_count;
}

ssize_t btStream::Flush() {
    return this->buffer_out.Flush(this->sock);
}

int btStream::GetOutBufferMaxSize() {
    return 0xFA000;
}

ssize_t btStream::Send_Piece(size_t i_piece_quality, size_t i_piece_id, block_t *p_piece) {
    size_t v4; // ST28_4@2
    char* v5; // ebx@2
    ssize_t result; // eax@2

    if ( p_piece )
    {
        print_wrn(goalbitp2p, "Sending piece %d with length %d (socket %d)", i_piece_id, p_piece->i_buffer_size, this->sock);
        v4 = p_piece->i_buffer_size;
        v5 = (char*)malloc(p_piece->i_buffer_size + 0x11);
        set_number((char *)v5, v4 + 0xD);
        *(_BYTE *)(v5 + 4) = 7;
        set_number((char *)(v5 + 5), i_piece_id);
        set_number((char *)(v5 + 9), i_piece_quality);
        set_number((char *)(v5 + 0xD), p_piece->i_buffer_size);
        this->buffer_out.Put(this->sock, (char *)v5, 0x11u);
        delete[](v5);
        result = this->buffer_out.PutAndFlush(this->sock, (char *)p_piece->p_buffer, p_piece->i_buffer_size);
    }
    else
    {
        result = -1;
    }
    return result;
}

ssize_t btStream::Send_Handshake(char *p_header, size_t i_header_length, size_t i_win_off,
                                 size_t i_win_length, int i_peer_quality) {

    ssize_t result = 0;
    if ( this->p_this->p_structure->b_p2p_enabled ) {
        print_wrn(goalbitp2p,
                  "[btStream] HANDSHAKE_OUT: i_header_length=%d, i_win_off=%d, i_win_length=%d, i_peer_quality=%d, %lld",
                  i_header_length,
                  i_win_off,
                  i_win_length,
                  i_peer_quality,
                  mdate());
        i_win_off = get_number((char*)&i_win_off);
        i_win_length = get_number((char*)&i_win_length);
        i_peer_quality = get_number((char*)&i_peer_quality);
        result = this->buffer_out.Put(this->sock, p_header, i_header_length);
        if ( result >= 0 )
        {
            result = this->buffer_out.Put(this->sock, (char *)&i_peer_quality, 4u);
            if ( result >= 0 )
            {
                result = this->buffer_out.Put(this->sock, (char *)&i_win_off, 4u);
                if ( result >= 0 )
                    result = this->buffer_out.Put(this->sock, (char *)&i_win_length, 4u);
            }
        }
    }
    return result;
}

unsigned int btStream::Send_Have(size_t i_piece_quality, size_t i_piece_id, size_t i_abi, size_t i_peer_quality) {
    unsigned int result; // eax@1
    char msg[21]; // [sp+17h] [bp-25h]@1

    set_number(msg, 0x11u);
    msg[4] = 4;
    set_number(&msg[5], i_piece_id);
    set_number(&msg[9], i_piece_quality);
    set_number(&msg[0xD], i_peer_quality);
    set_number(&msg[0x11], i_abi);
    print_wrn(goalbitp2p, "Send_Have  abi:%d piece id:%d piece quality:%d peer quality:%d", i_abi, i_piece_id, i_piece_quality, i_peer_quality);
    result = this->buffer_out.Put(this->sock, msg, 0x15u);
    return result;
}

unsigned int btStream::Send_WinUpdate(size_t i_piece_id) {
    char msg[9]; // [sp+13h] [bp-19h]@1
    set_number(msg, 5u);
    msg[4] = 0xA;
    set_number(&msg[5], i_piece_id);
    print_wrn(goalbitp2p, "Send_WinUpdate %d piece", i_piece_id);
    return this->buffer_out.Put(this->sock, msg, 9u);
}

btStream::btStream(goalbit_t_0 *p_goalbit) {
    this->sock = -1;
    this->sock_was = -1;
    this->m_oldbytes = 0;
    this->p_this = p_goalbit;
}

ssize_t btStream::Send_Keepalive() {
    size_t i = 0;
    print_wrn(goalbitp2p, "Send_Keepalive");
    return this->buffer_out.Put(this->sock, (char *)&i, 4u);
}

size_t btStream::RemoveMessage() {
    char *v1; // ebx@1
    size_t i_length; // [sp+18h] [bp-14h]@1
    size_t i_size; // [sp+1Ch] [bp-10h]@1

    i_length = 4;
    v1 = this->buffer_in.Get(&i_length);
    i_size = get_number(v1) + 4;
    this->buffer_in.Remove(&i_size);
    if ( v1 && i_length )
        free(v1);
    return i_size;
}

size_t btStream::GetInBufferLeftSize() {
    return this->buffer_in.i_buffer_size - this->buffer_in.i_buffer_count;
}

bool btStream::CanIncreaseInBuffer() {
    return this->buffer_in.i_buffer_size <= 0xF9FFF;
}

int Peer::ReceivePiece() {
    ssize_t v1; // esi@4
    int result; // eax@10
    Rate *v9; // eax@19
    const char *v10; // eax@22
    //int *v11; // eax@23

    if ( this->m_err_count > 0x1F )
    {
        this->_peerF28 &= 0b10111111;
        return -1;
    }
    v1 = 0;
    if ( this->stream->GetBufferedMessageType() == 7 )
    {
        if ( !p_this->p_structure->g_next_dn || this == p_this->p_structure->g_next_dn ) {
            //v7 = this->p_this;
            if ( p_this->p_structure->WORLD->BandWidthLimitDown(p_this->p_structure->Self->p_rate_dl->m_late) != 0 )
            {
                if ( !p_this->p_structure->g_next_dn )
                {
                    print_wrn(goalbitp2p, "[Peer] ReceivePiece - %p waiting for DL bandwidth", this);
                    this->p_this->p_structure->g_next_dn = this;
                }
            } else {
                p_this->p_structure->g_next_dn = 0;
                v1 = this->stream->Feed(this->p_rate_dl);
                v9 = this->p_this->p_structure->Self->p_rate_dl;
                v9->_bf40 &= 0b11111110;
                if (v1 < 0) {
                    v10 = "remote closed";
                    if (v1 != -2) {
                        v10 = (const char *) strerror(errno);
                    }
                    print_wrn(goalbitp2p, "[Peer] ReceivePiece - %p: %s", this, v10);
                    return -1;
                }
            }
        }
    }
    else if ( !this->stream->HasBufferedMessage() )
    {
        v1 = this->stream->Feed(this->p_rate_dl);
        if ( v1 < 0 )
        {
            v10 = "remote closed";
            if ( v1 != -2 )
            {
                v10 = (const char *)strerror(errno);
            }
            print_wrn(goalbitp2p, "[Peer] ReceivePiece - %p: %s", this, v10);
            return -1;
        }
    }
    while ( this->stream->HasBufferedMessage() )
    {
        if ( v1 < 0 )
            return -1;
        v1 = ReceiveMessage();
        if ( v1 == -2 )
        {
            print_wrn(goalbitp2p, "[Peer] ReceivePiece - %p seed<->seed detected", this);
            this->_peerF28 &= 0b10111111;
            return -1;
        }
        if ( v1 < 0 || this->stream->RemoveMessage() < 0 )
            return -1;
    }
    return 0;
}

unsigned int Peer::ReceiveMessage() {
    int v8; // eax@10
    const char *v10; // edx@29

    size_t i_msg_size; // [sp+2Ch] [bp-20h]@3

    if ( !this->stream->HasBufferedMessage() )
    {
        ssize_t v1 = this->stream->Feed(this->p_rate_dl);
        if ( v1 < 0 )
        {
            v10 = "remote closed";
            if ( v1 != -2 )
            {
                v10 = (const char *)strerror(errno);
            }
            print_msg(goalbitp2p, "%p: %s", this, v10);
            return -1;
        }
    }
    size_t v2 = this->stream->GetBufferedMessageLength();
    char v3 = this->stream->GetBufferedMessageType();
    i_msg_size = v2 + 5;
    char *v4 = this->stream->GetBufferedMessage(&i_msg_size);
    if ( !v4 ) {
        return -1;
    }
    this->m_last_timestamp = p_this->p_structure->now;
    if ( v2 )
    {
        switch ( v3 )
        {
            case 0xB:
                print_wrn(goalbitp2p, "[Peer] ReceiveMessage - M_EOS");
                v8 = ProcessEndOfStream(v2, v4);
                break;
            case 0:
                print_wrn(goalbitp2p, "[Peer] ReceiveMessage - M_CHOKE");
                v8 = ProcessChoke();
                break;
            case 1:
                print_wrn(goalbitp2p, "[Peer] ReceiveMessage - M_UNCHOKE");
                v8 = ProcessUnchoke();
                break;
            case 2:
                print_wrn(goalbitp2p, "[Peer] ReceiveMessage - M_INTERESTED");
                v8 = ProcessInterested(v2, v4);
                break;
            case 3:
                print_wrn(goalbitp2p, "[Peer] ReceiveMessage - M_NOT_INTERESTED");
                print_wrn(goalbitp2p, "[Peer] ProcessNotInterested - %p is not interested", this);
                this->m_state &= 0xFDu;
                v8 = 0;
            case 4:
                print_wrn(goalbitp2p, "[Peer] ReceiveMessage - M_HAVE");
                v8 = ProcessHave(v2, v4);
                break;
            case 5:
                print_wrn(goalbitp2p, "[Peer] ReceiveMessage - M_BITFIELD");
                v8 = ProcessBitfield(v2, v4);
                break;
            case 6:
                print_wrn(goalbitp2p, "[Peer] ReceiveMessage - M_REQUEST");
                v8 = ProcessRequest(v2, v4);
                break;
            case 7:
                print_wrn(goalbitp2p, "[Peer] ReceiveMessage - M_PIECE");
                v8 = ProcessPiece(v2, v4);
                break;
            case 8:
                print_wrn(goalbitp2p, "[Peer] ReceiveMessage - M_CANCEL");
                v8 = ProcessCancelPiece(v2, v4);
                break;
            case 0xA:
                print_wrn(goalbitp2p, "[Peer] ReceiveMessage - M_WIN_UPDATE");
                v8 = ProcessWindowsUpdate(v2, v4);
                break;
            default:
                print_wrn(goalbitp2p, "[Peer] ReceiveMessage -  Unknown message type %d from peer %p", v3, this);
                v8 = 0;
                break;
        }
        if ( v8 >= 0 ) {
            this->m_lastmsg = v3;
        }
        free(v4);
        return v8;
    }
    if ( _peerF28 & 1 || this->stream->Send_Keepalive() >= 0 )
    {
        _peerF28 = _peerF28 & 0b11111110;
        free(v4);
        return 0;
    }
    free(v4);
    return -1;
}

int Peer::RequestCheck() {
    int result; // eax@9
    int v5; // edx@14
    int v6; // edx@16
    size_t v7; // esi@18
    size_t v8; // eax@18
    size_t v11; // edi@18
    size_t v12; // esi@18
    int v13; // edx@23

    if ( !Need_Remote_Data() ) {
        if ( !this->p_request_q->Size() )
        {
            if ( this->m_state & 8 )
            {
                v5 = SetLocal(3);
                result = -1;
                if ( v5 < 0 )
                    return result;
            }
        }
        LABEL_8:
        if ( this->p_request_q->IsEmpty() )
        {
            this->p_rate_dl->StopTimer();
            result = 0;
        }
        else
        {
            this->p_rate_dl->StartTimer();
            result = 0;
        }
        return result;
    }
    if ( !(this->m_state & 8) )
    {
        v6 = SetLocal(2);
        if ( v6 < 0 )
            return -1;
    }
    if ( this->m_state & 1 )
        goto LABEL_8;
    //v2 = this->p_this;
    if ( this->m_req_out > p_this->p_structure->cfg_req_queue_length )
    {
        print_dbg(goalbitp2p, "[Peer] RequestCheck - ERROR@4: %p m_req_out underflow, resetting", this);
        this->m_req_out = 0;
    }
    if ( !this->p_request_q->IsEmpty() || (v13 = RequestPiece(), result = -1, v13 >= 0) )
    {
        if ( this->m_req_out < this->m_req_send )
        {
            v7 = this->p_request_q->GetHeadIndex();
            v8 = this->p_request_q->GetHeadQuality();
            v11 = this->p_request_q->GetRequestLength(v8, v7);
            v12 = this->p_rate_dl->RateMeasure();
            if ( (this->m_req_out <= 1
                  || !this->p_rate_dl->RateMeasure()
                  || (unsigned int)(__int64)((long double)((this->m_req_out + 1) * v11) / (long double)v12)
                     - this->m_latency <= 1)
                 && SendRequest() < 0 )
            {
                return -1;
            }
        }
        goto LABEL_8;
    }
    return result;
}

int Peer::Need_Remote_Data() {
    Mypthread_mutex_lock(93, &this->p_this->p_structure->exec_lock);
    int i_segment_id = p_this->p_structure->exec_info.i_segment_id;
    size_t i_quality = p_this->p_structure->exec_info.i_quality;
    Mypthread_mutex_unlock(93, &p_this->p_structure->exec_lock);

    size_t i_piece_id = this->p_this->p_structure->p_metadataManager->segmentID2PieceID(i_segment_id, i_quality, 3, 2);
    int i_p2p_max_piece = this->p_this->p_structure->p_metadataManager->getBiggestPieceID(i_quality);
    btBitField *v5 = new btBitField(this->bitfield);
    v5->Xor(this->p_this->p_structure->BTCONTENT->p_p2p_bitfield);
    if ( i_piece_id != 0x80000001 ) {
        v5->UnsetSmallerPieces(i_piece_id);
        v5->UnSet(i_piece_id);
    }
    if ( i_p2p_max_piece != 0x80000001 ) {
        v5->UnsetBiggerPieces(i_p2p_max_piece);
    }
    this->p_this->p_structure->WORLD->CheckP2PBitFieldFromP2P(v5);
    this->p_this->p_structure->BTCONTENT->CheckP2PBitFieldFromHLS(v5);
    int v6 = !v5->IsEmpty(i_quality);
    if ( v5 )
    {
        delete v5;
    }
    return v6;
}

int Peer::SetLocal(int s) {
    int result; // eax@5
    unsigned int v3; // eax@8
    //goalbit_t_0 *v4; // eax@8
    //goalbit_structure_t *v5; // eax@8
    PeerRequestQueue *v6; // eax@14
    unsigned int v7; // eax@20
    //goalbit_t_0 *v8; // eax@20

    if ( s == 1 )
    {
        if ( !this->p_reponse_q->IsEmpty() )
            this->p_rate_ul->StartTimer();
        result = 0;
        if ( this->m_state & 0b100 )
        {
            this->m_unchoke_timestamp = this->p_this->p_structure->now;
            v7 = this->p_rate_dl->RateMeasure();
            print_wrn(goalbitp2p,
                      "[Peer] SetLocal - Unchoking %p (D=%lluMB@%dK/s)",
                      this,
                      this->p_rate_dl->m_count_bytes >> 0x14,
                      v7 >> 0xA);
            //v8 = this->p_this;
            this->m_state &= 0xFBu;
            this->m_next_send_time = p_this->p_structure->now;
            return this->stream->Send_State(s);
        }
    }
    else if ( s < 1u )
    {
        result = 0;
        if ( !(this->m_state & 4) )
        {
            this->m_unchoke_timestamp = this->p_this->p_structure->now;
            v3 = this->p_rate_dl->RateMeasure();
            print_wrn(goalbitp2p,
                      "[Peer] SetLocal - Choking %p (D=%lluMB@%dK/s)",
                      this,
                      this->p_rate_dl->m_count_bytes >> 0x14,
                      v3 >> 0xA);
            //v4 = this->p_this;
            this->m_state |= 4u;
            //v5 = v4->p_structure;
            if ( p_this->p_structure->g_next_up == this )
                p_this->p_structure->g_next_up = 0;
            if ( !this->p_reponse_q->IsEmpty() )
                this->p_reponse_q->Clear();
            this->p_rate_ul->StopTimer();
            _bf29 &= 0xFBu;
            return this->stream->Send_State(s);
        }
    }
    else if ( s == 2 )
    {
        result = 0;
        if ( !(this->m_state & 8) )
        {
            print_wrn(goalbitp2p, "[Peer] SetLocal - Interested in %p", this);
            this->m_state |= 8u;
            return this->stream->Send_State(s);
        }
    }
    else
    {
        if ( s != 3 )
            return -1;
        result = 0;
        if ( this->m_state & 8 )
        {
            print_wrn(goalbitp2p, "[Peer] SetLocal - Not interested in %p", this);
            v6 = this->p_request_q;
            this->m_state &= 0xF7u;
            if ( !v6->IsEmpty() )
            {
                if ( CancelRequestedPieces() < 0 )
                    return -1;
                this->p_request_q->Clear();
            }
            return this->stream->Send_State(s);
        }
    }
    return result;
}

int Peer::CancelRequestedPieces() {
    PIECE *i; // ebx@1
    int result; // eax@10

    for ( i = this->p_request_q->GetHead(); i; this->m_cancel_time = p_this->p_structure->now )
    {
        while ( !i->t_request_time )
        {
            i = i->p_next;
            if ( !i )
                goto LABEL_8;
        }
        this->stream->Send_Cancel(i->i_quality, i->i_index);
        this->m_req_out--;
        if ( this->m_req_out > p_this->p_structure->cfg_req_queue_length )
        {
            print_err(goalbitp2p, "[Peer] CancelRequestedPieces - ERROR@2: %p m_req_out underflow, resetting", this);
            //v7 = this->p_this;
            this->m_req_out = 0;
            //v3 = v7->p_structure;
        }
        i = i->p_next;
    }
    LABEL_8:
    if ( this->m_req_out || (p_this->p_structure->g_next_dn != this) )
    {
        result = 0;
    }
    else
    {
        p_this->p_structure->g_next_dn = 0;
        result = 0;
    }
    return result;
}

unsigned int Peer::ProcessEndOfStream(size_t i_msg_length, char *p_msg_data) {
    unsigned int result; // eax@1
    size_t v4; // esi@2
    size_t v5; // esi@3
    size_t v6; // eax@3

    if ( !i_msg_length ) {
        return -1;
    }
    v4 = get_number(p_msg_data + 5);
    print_wrn(goalbitp2p, "[Peer] ProcessEndOfStream - %p tell us that the EOS is %u", this, v4);
    this->p_this->p_structure->i_eos_piece_id = v4;
    while ( 1 )
    {
        v5 = this->p_request_q->GetHeadIndex();
        v6 = this->p_request_q->GetHeadQuality();
        if ( !v5 )
            break;
        if ( this->p_this->p_structure->i_eos_piece_id < v5 )
        {
            this->p_request_q->Remove(v6, v5);
            --this->m_req_out;
        }
    }
    if ( !this->m_req_out )
    {
        if ( p_this->p_structure->g_next_dn == this )
            p_this->p_structure->g_next_dn = 0;
    }
    return 0;
}


int Peer::ProcessChoke() {
    Rate *v2; // eax@2
    PeerRequestQueue *v4; // eax@5
    size_t v6; // eax@8

    print_dbg(goalbitp2p, "[Peer] ProcessChoke -  %p choked me", this);
    int v1 = this->m_last_timestamp;
    if ( this->m_lastmsg == 1 && v1 <= this->m_choketime + 1 )
    {
        v6 = this->m_err_count + 2;
        this->m_err_count = v6;
        print_err(goalbitp2p, "[Peer] ProcessChoke - Err: %p (%d) Choke oscillation", this, v6);
        v1 = this->m_last_timestamp;
    }
    this->m_choketime = v1;
    v2 = this->p_rate_dl;
    this->m_state |= 1u;
    v2->StopTimer();
    //v3 = this->p_this->p_structure;
    if ( p_this->p_structure->g_next_dn == this )
        p_this->p_structure->g_next_dn = 0;
    if ( !this->p_request_q->IsEmpty() )
    {
        v4 = this->p_request_q;
        this->m_req_out = 0;
        this->p_this->p_structure->PENDINGQUEUE->AddPieces(v4);
    }
    return 0;
}

int Peer::ProcessUnchoke() {
    size_t v4; // eax@3

    print_dbg(goalbitp2p, "[Peer] ProcessUnchoke - %p unchoked me", this);
    int v2 = this->m_last_timestamp;
    if ( !this->m_lastmsg && v2 <= this->m_choketime + 1 )
    {
        v4 = this->m_err_count + 2;
        this->m_err_count = v4;
        print_dbg(goalbitp2p, "[Peer] ProcessUnchoke - Err: %p (%d) Choke oscillation", this, v4);
        v2 = this->m_last_timestamp;
    }
    this->m_choketime = v2;
    this->m_state &= 0xFEu;
    this->_bf29 &= 0xE7u;
    return RequestCheck();
}

unsigned int Peer::RequestPiece() {
    //goalbit_structure_t *v3; // edx@1
    unsigned int result; // eax@2
    //goalbit_structure_t *v5; // eax@5
    size_t quality; // edi@5
    int segment_id; // esi@5
    size_t v8; // ebp@5
    btBitField *v9; // esi@5
    int v10; // eax@14
    int v11; // esi@15
    array_t<metadata_piece_t> *v12; // ebp@15
    metadata_piece_t *v14; // ebx@17
    btSelfBitField *v17; // eax@18
    bool v18; // dl@24
    int i_p2p_max_piece; // [sp+28h] [bp-24h]@5
    int i_piece_id; // [sp+2Ch] [bp-20h]@12

    if ( this->p_request_q->Size() >= p_this->p_structure->cfg_req_queue_length )
    {
        this->m_req_send = this->m_req_out;
        return 0;
    }
    if ( p_this->p_structure->PENDINGQUEUE->GetPieces(this->p_request_q, this->bitfield) )
    {
        print_dbg(goalbitp2p, "[Peer] RequestPiece - Assigning to %p from Pending", this);
        return this->SendRequest();
    }

    Mypthread_mutex_lock(94, &this->p_this->p_structure->exec_lock);
    quality = p_this->p_structure->exec_info.i_quality;
    segment_id = p_this->p_structure->exec_info.i_segment_id;
    Mypthread_mutex_unlock(94, &p_this->p_structure->exec_lock);

    v8 = this->p_this->p_structure->p_metadataManager->segmentID2PieceID(segment_id, quality, 3, 2);
    i_p2p_max_piece = this->p_this->p_structure->p_metadataManager->getBiggestPieceID(quality);
    v9 = new btBitField(this->bitfield);
    //btBitField::btBitField(v9, this->bitfield);
    v9->Xor(this->p_this->p_structure->BTCONTENT->p_p2p_bitfield);
    if ( v8 != 0x80000001 )
    {
        v9->UnsetSmallerPieces(v8);
        v9->UnSet(v8);
    }
    if ( i_p2p_max_piece != 0x80000001 )
        v9->UnsetBiggerPieces(i_p2p_max_piece);
    this->p_this->p_structure->WORLD->CheckP2PBitFieldFromP2P(v9);
    this->p_this->p_structure->BTCONTENT->CheckP2PBitFieldFromHLS(v9);
    if ( !v9->IsEmpty(quality) )
    {
        i_piece_id = v9->SelectRandomUniformID(quality);
        if ( v9 )
        {
            delete v9;
        }
        if (i_piece_id == 0) {
            return 0;
        }
        print_dbg(goalbitp2p, "[Peer] RequestPiece - Assigning #(%u,%u) to %p", i_piece_id, quality, this);
        v10 = this->p_this->p_structure->p_metadataManager->getPieceSegment(i_piece_id);
        if ( v10 != 0x80000001 )
        {
            v11 = 0;
            v12 = this->p_this->p_structure->p_metadataManager->getSegmentPieces(quality, v10);
            if ( v12 )
            {
                while ( v11 < array_count(v12) )
                {
                    v14 = array_item_at_index(v12, v11);
                    if ( v14 ) {
                        v17 = p_this->p_structure->BTCONTENT->p_p2p_bitfield;
                        if ( v17 )
                        {
                            if ( v17->IsSet(quality, v14->i_piece_id) )
                            {
                                freePiece(v14);
                                v11++;
                                continue;
                            }
                        }
                        if ( !(unsigned __int8)p_this->p_structure->PENDINGQUEUE->ExistPiece(quality, v14->i_piece_id)
                             && !this->p_this->p_structure->WORLD->AlreadyRequested(quality, v14->i_piece_id)
                             && !this->p_this->p_structure->p_hlsManager->isPieceRequestedOrReady(

                                v14->i_piece_id)
                             && i_piece_id != v14->i_piece_id )
                        {
                            print_dbg(goalbitp2p,
                                      "[Peer] RequestPiece - Adding #(%u,%u) to the pending queue",
                                      v14->i_piece_id,
                                      quality);
                            this->p_this->p_structure->PENDINGQUEUE->AddPiece(quality, v14->i_piece_id);
                        }
                        freePiece(v14);
                    }
                    ++v11;
                }
                array_destroy(v12);
            }
        }
        v18 = this->p_request_q->Add(quality, i_piece_id, 0, 0);
        if ( !v18 )
            return -1;
        return this->SendRequest();
    }
    if ( v9 )
    {
        delete v9;
    }
    return 0;
}

unsigned int Peer::SendRequest() {
    unsigned int result; // eax@4
    int v4; // edi@6
    signed int v5; // ebp@6
    double v7; // ST20_8@18

    if ( this->m_req_out > p_this->p_structure->cfg_req_queue_length ) {
        print_msg(goalbitp2p, "[Peer] SendRequest - ERROR@5: %p m_req_out underflow, resetting", this);
        this->m_req_out = 0;
    }
    PIECE *v2 = this->p_request_q->GetSend();
    if ( !v2 )
        goto LABEL_14;
    result = 0;
    if ( this->m_req_out >= this->m_req_send )
        return result;
    v4 = 0;
    v5 = 1;
    print_dbg(goalbitp2p, byte_8171E59);
    print_dbg(goalbitp2p,
              "[Peer] SendRequest - Requesting #(%u,%u) from %p:",
              v2->i_index,
              v2->i_quality,
              this);
    if ( this->m_req_send <= this->m_req_out )
    {
        LABEL_13:
        print_dbg(goalbitp2p, byte_8171E59);
        this->m_receive_time = this->p_this->p_structure->now;
        LABEL_14:
        if ( this->m_req_out >= this->m_req_send )
            result = 0;
        else
            result = RequestPiece();
        return result;
    }
    while ( 1 )
    {
        if ( !v5
             || this->p_rate_dl->RateMeasure()
                && (v7 = (long double)(v2->i_length * (this->m_req_out + 1)),
                v7 / (long double)this->p_rate_dl->RateMeasure() - (long double)this->m_latency > 0.0) )
        {
            this->p_request_q->SetRequestTime(v2, 0);
        }
        else
        {
            v5 = 0;
            this->p_request_q->SetRequestTime(v2, this->p_this->p_structure->now);
        }
        if ( this->stream->Send_Request(v2->i_quality, v2->i_index) < 0 )
            return -1;
        this->p_request_q->SetSend(v2->p_next);
        v2 = v2->p_next;
        this->m_req_out++;
        if ( v2 )
        {
            if ( ++v4 <= 4 && this->m_req_send > this->m_req_out )
                continue;
        }
        goto LABEL_13;
    }
}

char *hexencode(const unsigned char *data, size_t length, char *dstbuf)
{
    static char hexdigit[17] = "0123456789ABCDEF";
    const unsigned char *src, *end;
    char *dst;

    if( 0==length ) length = strlen((char *)data);
    end = data + length;
    if( !dstbuf ) dstbuf = new char[length * 2 + 1];
    dst = dstbuf;

    if( dst ){
        for( src = data; src < end; src++ ){
            *dst++ = hexdigit[*src >> 4];
            *dst++ = hexdigit[*src & 0x0f];
        }
        *dst = '\0';
    }
    return dstbuf;
}

void TextPeerID(const unsigned char *peerid, char *txtid)
{
    int i, j;

    for( i = j = 0; i < PEER_ID_LEN; i++ ){
        if( i == j && isprint(peerid[i]) && !isspace(peerid[i]) )
            txtid[j++] = peerid[i];
        else{
            strcpy(txtid + j, "0x");
            j += 2;
            break;
        }
    }
    if( i < PEER_ID_LEN )
        hexencode(peerid + i, PEER_ID_LEN - i, txtid + j);
    else txtid[j] = '\0';
}

int Peer::ReceiveHandShake() {
    int v3; // edi@2
    unsigned __int8 *v9; // ebp@6
    signed int v12; // edi@9
    int v13; // eax@10
    btContent *v14; // eax@11
    int v17; // edi@19
    int v18; // edx@20
    int v19; // eax@22
    btSelfBitField *v24; // eax@26
    const char *v26; // edx@31
    size_t *v29; // edi@34
    char *v32; // ebp@35
    btContent* v33; // [sp+28h] [bp-74h]@6
    size_t i_win_length; // [sp+28h] [bp-74h]@34
    size_t i_offset; // [sp+2Ch] [bp-70h]@34
    size_t i_sent; // [sp+38h] [bp-64h]@2
    char txtid[43]; // [sp+3Dh] [bp-5Fh]@13
    unsigned __int8 this_id[20]; // [sp+68h] [bp-34h]@13

    ssize_t v1 = this->stream->Feed();
    if ( v1 < 0 )
    {
        v26 = "remote closed";
        if ( v1 != -2 )
        {
            v26 = (const char *)strerror(errno);
        }
        print_wrn(goalbitp2p, "[Peer] ReceiveHandShake - %p: %s(%s:%hu)", this, v26, inet_ntoa(m_sin.sin_addr), ntohs(m_sin.sin_port) );
        return -1;
    }
    i_sent = 0x4D;
    if ( this->stream->GetInBufferCount() < 0x4D ) {
        return 0;
    }
    char *v6 = this->stream->GetBufferedMessage(&i_sent);
    print_wrn(goalbitp2p,
              "[Peer] ReceiveHandShake - i_header_length=%d, i_length=%d, p_data=%s %lld",
              0x4D,
              i_sent,
              v6,
              mdate());
    if ( i_sent <= 0x4C ) {
        if (v6) {
            free(v6);
        }
        return -1;
    }
    if ( v6 )
    {
        v9 = p_this->p_structure->BTCONTENT->m_shake_buffer;
        v33 = p_this->p_structure->BTCONTENT;
        if ( !memcmp(v6, v9, 0x41u) )
        {
            print_wrn(goalbitp2p, "[Peer] ReceiveHandShake - peer %p is myself", this);
            free(v6);
            return -1;
        }
        if ( memcmp(v6, v9, 0x11u) )
        {
            print_wrn(goalbitp2p, "[Peer] ReceiveHandShake - Peer %p speeks a different protocol: close handshake", this);
            free(v6);
            return -1;
        }
        if ( memcmp(v6 + 0x10, &v33->m_shake_buffer[0x11], 8u) )
        {
            v12 = 0x10;
            print_dbg(goalbitp2p, byte_8171E59);
            print_dbg(goalbitp2p, "[Peer] ReceiveHandShake - peer %p gave 0x", this);
            do
            {
                v13 = (unsigned __int8)v6[v12++];
                print_dbg(goalbitp2p, "%2.2hx", v13);
            } while ( v12 != 0x18 );
            print_dbg(goalbitp2p, "[Peer] ReceiveHandShake - as reserved bytes");
            v14 = this->p_this->p_structure->BTCONTENT;
            memcpy(v6+0x11, v14->m_shake_buffer+0x11, 8);
            //v8 = this->p_this;
            v9 = p_this->p_structure->BTCONTENT->m_shake_buffer;
        }
        if ( memcmp(v6, v9, 0x2Du) )
        {
            //v16 = v8;
            v17 = 0;
            print_dbg(goalbitp2p, byte_8171E59);
            print_dbg(goalbitp2p, "mine: 0x");
            do
            {
                v18 = this->p_this->p_structure->BTCONTENT->m_shake_buffer[v17++];
                print_dbg(goalbitp2p, "%2.2hx", v18);
            } while ( v17 != 0x2D );
            v17 = 0;
            print_dbg(goalbitp2p, byte_8171E59);
            print_dbg(goalbitp2p, "peer: 0x");
            do {
                v19 = (unsigned __int8)v6[v17++];
                print_dbg(goalbitp2p, "%2.2hx", v19);
            } while ( v17 != 0x2D );
            free(v6);
            return -1;
        }
    }
    memcpy(this_id, v6+0x2D, 20);
    TextPeerID((const unsigned char *)v6 + 0x2D, txtid);
    print_wrn(goalbitp2p, "[Peer] ReceiveHandShake - Peer %p ID: %s", this, txtid);
    if ( this->p_this->p_structure->WORLD->AlreadyHaveThisPeer(this_id) )
    {
        print_wrn(goalbitp2p, "[Peer] ReceiveHandShake - I already have peer %p", this);
        if ( v6 )
            free(v6);
        return -1;
    }
    memcpy(id, this_id, 20);
    print_wrn(goalbitp2p, "[Peer] ReceiveHandShake - Peer %p has quality %d", this, get_number(v6 + 0x41));
    this->i_win_offset = get_number(v6 + 0x45);
    print_wrn(goalbitp2p, "[Peer] ReceiveHandShake - Peer %p window offset: %u", this, this->i_win_offset);
    this->i_win_length = get_number(v6 + 0x49);
    print_wrn(goalbitp2p, "[Peer] ReceiveHandShake - Peer %p window length: %u", this, this->i_win_length);
    if ( v6 )
        free(v6);
    v24 = this->p_this->p_structure->BTCONTENT->p_p2p_bitfield;
    if ( v24 && !v24->IsEmpty( 0x80000001) )
    {
        v29 = (size_t *)malloc(sizeof(size_t *));
        *v29 = 0;
        i_offset = this->i_win_offset;
        i_win_length = this->i_win_length;
        this->p_this->p_structure->BTCONTENT->p_p2p_bitfield->print();
        if ( this->p_this->p_structure->BTCONTENT->p_p2p_bitfield )
        {
            v32 = this->p_this->p_structure->BTCONTENT->p_p2p_bitfield->GetBitfieldData(i_offset, i_win_length, v29);
            if ( v32 )
            {
                if ( *v29 )
                    i_sent = this->stream->Send_Bitfield(this->i_win_offset, *v29, v32);
                free(v32);
            }
        }
        delete(v29);
    }
    if ( this->stream->RemoveBytes(0x4Du) >= 0 )
    {
        _bf29 &= 0b11111110;
        _peerF28 = (this->_peerF28 & 0b11100001) | 4;
        if ( this->stream->HasBufferedMessage() )
            i_sent = ReceivePiece();
        return i_sent;
    }
    return -1;
}

void Peer::CloseConnection() {
    //goalbit_structure_t *v2; // eax@3

    print_wrn(goalbitp2p, "[Peer] CloseConnection - %p closed", this);
    if ( (_peerF28 & 0b11110u) != 0b110u) {
        this->_peerF28 = (_peerF28 & 0b11100001u) | 0b110u;
        this->p_rate_dl->StopTimer();
        this->p_rate_ul->StopTimer();
        this->stream->Close();
    }
    if (!this->p_request_q->IsEmpty()) {
        this->p_this->p_structure->PENDINGQUEUE->AddPieces(this->p_request_q);
        //v2 = this->p_this->p_structure;
    }
    if ( p_this->p_structure->g_next_up == this ) {
        p_this->p_structure->g_next_up = 0;
    }
    if (p_this->p_structure->g_next_dn == this) {
        p_this->p_structure->g_next_dn = 0;
    }
}

int Peer::SendPiece() {
    //goalbit_structure_t *v2; // eax@3
    int result; // eax@5
    //int *v4; // eax@8
    //int *v5; // esi@8
    //int v6; // eax@8
    int v7; // eax@10
    //goalbit_t_0 *v8; // ecx@10
    Peer *v9; // edx@10
    PeerList *v10; // eax@13
    Rate *v11; // eax@19
    //goalbit_structure_t *v12; // eax@21

    if ( this->stream->GetOutBufferCount() && this->stream->Flush() < 0 )
    {
        //v4 = _errno_location();
        //v5 = v4;
        print_err(goalbitp2p, "[Peer] SendPiece - %p: Network Error: %d (%s)", this, errno, strerror(errno));
        return -1;
    }
    if ( this->p_reponse_q->IsEmpty() || !CouldReponsePiece() )
    {
        //v2 = this->p_this->p_structure;
        if ( p_this->p_structure->g_next_up == this ) {
            p_this->p_structure->g_next_up = 0;
        }
    } else {
        v7 = this->p_this->p_structure->WORLD->BandWidthLimitUp(
                this->p_this->p_structure->Self->p_rate_ul->m_late);
        //v8 = this->p_this;
        v9 = p_this->p_structure->g_next_up;
        if ( v9 && this != v9 )
        {
            if ( !v7 )
            {
                print_dbg(goalbitp2p, "[Peer] SendPiece - %p deferring UL to %p", this, p_this->p_structure->g_next_up);
                v10 = this->p_this->p_structure->WORLD;
                ++v10->m_defer_count;
            }
        }
        else if ( v7 )
        {
            if ( !v9 )
            {
                print_dbg(goalbitp2p, "[Peer] SendPiece - %p waiting for UL bandwidth", this);
                //v12 = this->p_this->p_structure;
                p_this->p_structure->g_next_up = this;
                ++p_this->p_structure->WORLD->m_defer_count;
            }
        }
        else
        {
            if ( v9 )
                p_this->p_structure->g_next_up = 0;
            this->p_rate_ul->StartTimer();
            this->p_this->p_structure->Self->p_rate_ul->StartTimer();
            if ( ResponsePiece() < 0 )
                return -1;
            v11 = this->p_this->p_structure->Self->p_rate_ul;
            v11->_bf40 &= 0b11111110;
        }
    }
    result = 0;
    if ( !(this->m_state & 1) )
        result = RequestCheck();
    return result;
}

int Peer::CouldReponsePiece() {
    bool v1; // al@1
    int v2; // edx@1
    size_t v4; // esi@2
    size_t v5; // eax@2
    size_t v6; // edi@2
    size_t v7; // esi@2

    v1 = this->p_reponse_q->IsEmpty();
    v2 = 0;
    if ( !v1 )
    {
        this->p_reponse_q->GetHeadIndex();
        v4 = this->p_reponse_q->GetHeadIndex();
        v5 = this->p_reponse_q->GetHeadQuality();
        v6 = this->p_reponse_q->GetRequestLength(v5, v4) + 0x11;
        v7 = this->stream->GetOutBufferMaxSize();
        v2 = v6 <= v7 - this->stream->GetOutBufferCount();
    }
    return v2;
}

int Peer::ResponsePiece() {
    size_t v1; // edi@1
    time_t v2; // esi@1
    //goalbit_t_0 *v3; // ebp@1
    int v4; // ecx@1
    BasicPeer *v5; // esi@1
    __int64 v6; // ST48_8@1
    unsigned int v8; // esi@2
    ssize_t v9; // ebp@2
    //goalbit_t_0 *v10; // eax@3
    unsigned int v11; // ebp@5
    //goalbit_t_0 *v12; // eax@5
    time_t v13; // ecx@6
    unsigned int v14; // eax@6
    long double v15; // fst7@10
    long double v16; // fst6@10
    PeerList *v17; // eax@10
    Rate *v19; // eax@4
    signed int v20; // eax@15
    signed int v21; // ebp@15
    int v22; // ecx@17
    unsigned int v23; // ecx@19
    signed int v24; // [sp+34h] [bp-48h]@15
    size_t i_piece_quality; // [sp+38h] [bp-44h]@1
    //goalbit_t_0 *v26; // [sp+3Ch] [bp-40h]@15
    timespec nowspec; // [sp+54h] [bp-28h]@8
    block_t* pblock; // [sp+5Ch] [bp-20h]@1

    v1 = this->p_reponse_q->GetSmallestIndex();
    i_piece_quality = this->p_reponse_q->GetSmallestQuality();
    v2 = this->p_reponse_q->GetRequestTime(i_piece_quality, v1);
    time(&this->p_this->p_structure->now);
    //v3 = this->p_this;
    v4 = p_this->p_structure->now - v2;
    v5 = p_this->p_structure->Self;
    v6 = (__int64)((long double)(unsigned int)v4 * 0.5 + 0.5 * (long double)v5->m_latency);
    v5->m_latency = v6;
    print_dbg(goalbitp2p, "[Peer] ResponsePiece - Our current latency: %u, our global latency: %u", v4, (_DWORD)v6);
    pblock = this->p_this->p_structure->BTCONTENT->ReadPiece(i_piece_quality, v1);
    if ( pblock )
    {
        v8 = pblock->i_buffer_size;
        v9 = 0xFFFFFFFF;
        if ( pblock->i_buffer_size )
        {
            //v10 = this->p_this;
            if ( p_this->p_param->i_cache_size )
            {
                v19 = p_this->p_structure->Self->p_rate_ul;
                v19->_bf40 &= 0b11111110;
            }
            v11 = this->p_rate_ul->CurrentRate();
            print_msg(goalbitp2p, "[Peer] ResponsePiece - Sending (#%u,%u) to %p", v1, i_piece_quality, this);
            //v12 = this->p_this;
            if ( v11 )
            {
                v13 = p_this->p_structure->now;
                v14 = p_this->p_param->i_max_bandwidth_up;
                if ( v14 <= v11 && v14 )
                    v11 = v14;
                this->m_next_send_time = v13 + v8 / v11;
            }
            else
            {
                v24 = p_this->p_structure->Self->p_rate_ul->RateMeasure();
                v20 = this->p_this->p_structure->WORLD->GetUnchoked();
                //v26 = this->p_this;
                v21 = v20;
                if ( v20 <= 0 )
                    v21 = 1;
                v22 = this->p_this->p_param->i_max_bandwidth_up;
                if ( v22 )
                {
                    if ( v22 - v24 <= v22 / v21 )
                        v23 = (v21 + v22 - 1) / v21;
                    else
                        v23 = v22 - v24;
                    this->m_next_send_time = p_this->p_structure->now + v8 / v23;
                }
                else if ( v24 )
                {
                    this->m_next_send_time = p_this->p_structure->now + v8 / (unsigned int)(v24 / v21);
                }
                else
                {
                    this->m_next_send_time = p_this->p_structure->now;
                }
            }
            this->m_prefetch_time = 0;
            clock_gettime(_CLOCK_REALTIME, &nowspec);
            if ( nowspec.tv_sec != this->p_this->p_structure->now )
            {
                nowspec.tv_sec = this->p_this->p_structure->now;
                nowspec.tv_nsec = 0;
            }
            v9 = this->stream->Send_Piece(i_piece_quality, v1, pblock);
            block_Release(pblock);
            v15 = (long double)nowspec.tv_sec;
            v16 = (long double)nowspec.tv_nsec / 1000000000.0;
            v17 = this->p_this->p_structure->WORLD;
            ++v17->m_upload_count;
            this->p_rate_ul->CountAdd(v8);
            this->p_rate_ul->RateAdd(v8, this->p_this->p_param->i_max_bandwidth_up, v15 + v16);
            this->p_reponse_q->Remove(i_piece_quality, v1);
        }
    }
    else
    {
        v9 = -1;
    }
    return v9;
}

bool Peer::CheckSendStatus() {
    Peer *v2 = p_this->p_structure->g_next_up;
    if ( v2 == this )
    {
        if ( p_this->p_structure->WORLD->BandWidthLimitUp(p_this->p_structure->Self->p_rate_ul->m_late) )
        {
            v2 = v2->p_this->p_structure->g_next_up;
        }
        else
        {
            print_dbg(goalbitp2p, "[Peer] CheckSendStatus - %p is not write-ready", v2);
            //v4 = v2->p_this;
            v2 = 0;
            p_this->p_structure->g_next_up = 0;
        }
    }
    return v2 != 0;
}

int Peer::HealthCheck() {

    size_t v5; // esi@6
    int v6; // ecx@6
    int v8; // esi@11

    if ( p_this->p_structure->now - 0x3B <= this->m_health_time )
    {
        return 0;
    }
    this->m_health_time = p_this->p_structure->now;
    if ( (this->m_state & 1) != 0 || !this->m_req_out )
        goto LABEL_17;
    v5 = this->m_latency;
    v6 = 0x12C;
    if ( v5 )
    {
        v6 = 0x3C;
        if ( v5 >= 0x1E )
            v6 = 2 * v5;
    }
    if ( this->m_receive_time < (unsigned int)(p_this->p_structure->now - v6) )
    {
        if ( _peerF28 & 0x20
             || (_peerF28 = _peerF28 | 0x20,
                print_dbg(goalbitp2p, "[Peer] HealthCheck - %p unresponsive; resetting request queue", this),
                 v8 = CancelRequestedPieces(),
                this->p_this->p_structure->PENDINGQUEUE->AddPieces(this->p_request_q),
                this->m_req_out = 0,
                v8 < 0) )
        {
            return -1;
        }
        return RequestCheck();
    }
    LABEL_17:
    _peerF28 &= 0xDFu;
    return 0;
}

int Peer::Send_ShakeInfo() {

    size_t v1 = 0x80000001;
    btContent *v5 = p_this->p_structure->BTCONTENT;
    btSelfBitField *v6 = v5->p_p2p_bitfield;
    if ( v6 )
        v1 = v6->i_min_id;
    return this->stream->Send_Handshake((char*)v5->m_shake_buffer, sizeof(v5->m_shake_buffer), v1, p_this->p_structure->i_p2p_win_length, p_this->p_structure->i_target_quality);
}

int Peer::CancelPiece(size_t i_piece_id) {

    int result; // eax@7

    for ( PIECE *i = this->p_request_q->GetHead(); i; i = this->p_request_q->GetHead() )
    {
        while ( i->i_index != i_piece_id )
        {
            i = i->p_next;
            if ( !i )
                goto LABEL_5;
        }
        print_dbg(goalbitp2p, "[Peer] CancelPiece - Cancelling (#%u,*) to %p", i_piece_id, this);
        if ( i->t_request_time )
        {
            this->stream->Send_Cancel(i->i_quality, i->i_index);
            this->m_req_out--;
            if ( this->m_req_out > p_this->p_structure->cfg_req_queue_length )
            {
                print_err(goalbitp2p, "[Peer] CancelPiece - ERROR@1: %p m_req_out underflow, resetting", this);
                //v8 = this->p_this;
                this->m_req_out = 0;
                //v6 = v8->p_structure;
            }
            this->m_cancel_time = p_this->p_structure->now;
        }
        this->p_request_q->Remove(i->i_quality, i->i_index);
    }
    LABEL_5:
    if ( this->m_req_out || (p_this->p_structure->g_next_dn != this) )
    {
        result = 0;
    }
    else
    {
        p_this->p_structure->g_next_dn = 0;
        result = 0;
    }
    return result;
}

Peer::Peer(goalbit_t_0 *p_goalbit) : BasicPeer(p_goalbit) {

    this->bitfield = new btBitField(p_this, p_this->p_structure->i_hls_qualities, 0);
    this->stream = new btStream(this->p_this);
    memset(this->id, 0, sizeof(this->id));
    this->p_request_q = new PeerRequestQueue(this->p_this);
    this->p_reponse_q = new PeerRequestQueue(this->p_this);
    this->p_rate_dl = new Rate(this->p_this);
    this->p_rate_ul = new Rate(this->p_this);
    this->m_unchoke_timestamp = 0;
    this->m_state = 0b101;
    this->_peerF28 = 0;
    this->m_err_count = 0;
    this->m_next_send_time = p_this->p_structure->now;
    this->m_last_timestamp = p_this->p_structure->now;
    this->m_choketime = p_this->p_structure->now;
    this->m_receive_time = p_this->p_structure->now;
    this->m_health_time = p_this->p_structure->now;
    this->m_req_send = 5;
    this->m_req_out = 0;
    this->m_latency = 0;
    this->_bf29 = 0;
    this->m_prev_dlrate = 0;
    this->m_latency_timestamp = 0;
    this->m_cancel_time = 0;
    this->m_prefetch_time = 0;
    this->p_rate_dl->m_selfrate = p_this->p_structure->Self->p_rate_dl;
    p_rate_ul->m_selfrate = p_this->p_structure->Self->p_rate_ul;
    this->i_win_offset = 0;
    this->i_win_length = 0;
    this->i_ab_index = 0;
    this->i_peer_quality = 0;
}

bool Peer::NeedPrefetch() {
    bool result = 0;
    if ( (_peerF28 & 0b11110) == 4 )
    {
        result = 1;
        if ( this->m_state & 4 )
        {
            result = 0;
            if ( !(this->m_state & 1) )
                result = ((_bf29 >> 3) & 3) <= 1;
        }
    }
    return result;
}

void Peer::Prefetch(time_t deadline) {
    unsigned int v4; // ebp@5
    //goalbit_t_0 *v5; // eax@7
    //goalbit_structure_t *v7; // ecx@7
    signed int v8; // edx@7
    time_t v9; // edx@8
    Peer *v10; // eax@9
    time_t v11; // esi@11
    time_t v12; // eax@17
    size_t v13; // esi@18
    size_t v14; // eax@18
    size_t self_ru; // [sp+18h] [bp-44h]@5
    goalbit_param_t_0 *v16; // [sp+1Ch] [bp-40h]@7
    block_t *p_piece; // [sp+3Ch] [bp-20h]@18

    if ( (_peerF28 & 0b11110) == 4 )
    {//send request
        if ( this->m_state & 4 )
        {
            if ( this->m_state & 1 )
                return;
            if ( ((_bf29 >> 3) & 3) > 1 )
                return;
        }
        _bf29 = (_bf29 & 0xE70) | 0x10;
        v4 = this->p_this->p_structure->Self->p_rate_dl->RateMeasure();
        self_ru = this->p_this->p_structure->Self->p_rate_ul->RateMeasure();
        this->p_rate_dl->RateMeasure();
        this->p_rate_ul->RateMeasure();
        if ( !(this->m_state & 4) && !this->p_reponse_q->IsEmpty() )
        {
            v16 = p_this->p_param;
            v8 = v16->i_max_bandwidth_up;
            if ( v8 )
                v9 = (signed int)((long double)p_this->p_structure->Self->p_rate_ul->m_last_size / (long double)v8
                                  + p_this->p_structure->Self->p_rate_ul->m_last_realtime);
            else
                v9 = p_this->p_structure->now;
            v10 = p_this->p_structure->g_next_up;
            if ( v10 )
            {
                if ( this != v10 )
                    return;
                this->m_next_send_time = v9;
                v11 = v9;
            }
            else
            {
                v11 = this->m_next_send_time;
            }
            if ( v11 < v9 )
                v11 = v9;
            if ( v11 < deadline && (!v4 || v11 <= p_this->p_structure->now + (v16->i_cache_size << 0x14) / v4) )
            {
                v12 = this->m_prefetch_time;
                if ( !v12 || !(v4 | self_ru) || p_this->p_structure->now - v12 > p_this->p_structure->BTCONTENT->p_cache->i_cache_size / (self_ru + v4) )
                {
                    v13 = this->p_reponse_q->GetHeadIndex();
                    v14 = this->p_reponse_q->GetHeadQuality();
                    p_piece = this->p_this->p_structure->BTCONTENT->ReadPiece(v14, v13);
                    if ( p_piece )
                        block_Release(p_piece);
                    this->m_prefetch_time = this->p_this->p_structure->now;
                }
            }
        }
    }
}

int Peer::AreYouOK() {
    _peerF28 |= 1u;
    return this->stream->Send_Keepalive();
}

int Peer::Need_Local_Data() {
    int result = 0;
    if ( this->m_state & 2 )
    {
        btSelfBitField *v2 = this->p_this->p_structure->BTCONTENT->p_p2p_bitfield;
        btBitField *v3 = new btBitField(v2);
        v3->Xor(this->bitfield);
        result = !v3->IsEmpty(0x80000001);
        delete v3;
    }
    return result;
}

int Peer::NeedRead(int limited) {
    signed int v2; // esi@1
    Peer *v4; // eax@4

    v2 = 1;
    if ( (_peerF28 & 0b11110) == 4 && this->stream->GetBufferedMessageType() == 7 ) {
        if ( !this->p_this->p_structure->g_next_dn || (v2 = 0, this == this->p_this->p_structure->g_next_dn) ) {
            v2 = limited == 0;
        }
    }
    return v2;
}

int Peer::NeedWrite(int limited) {
    signed int v2; // esi@1
    size_t v6; // edi@9
    size_t v7; // esi@10
    size_t v8; // ebp@10
    size_t v9; // eax@10
    long double v10; // fst7@10

    v2 = 1;
    if ( !this->stream->GetOutBufferCount() && _peerF28 & 0b11110 )
    {
        if ( !(this->m_state & 4) )
        {
            if ( !this->p_reponse_q->IsEmpty() && !limited )
                return v2;
        }
        if ( (this->m_state & 9) != 8 || (v2 = 1, !this->p_request_q->IsEmpty()) )
        {
            v2 = 0;
            if ( this->p_request_q->GetSend() )
            {
                v2 = 0;
                if ( this->m_req_out < this->m_req_send )
                {
                    v2 = 1;
                    if ( this->m_req_out > 1 )
                    {
                        v6 = this->p_rate_dl->RateMeasure();
                        if ( v6 )
                        {
                            v7 = this->m_req_out;
                            v8 = this->p_request_q->GetHeadIndex();
                            v9 = this->p_request_q->GetHeadQuality();
                            v10 = (long double)((v7 + 1) * this->p_request_q->GetRequestLength(v9, v8));
                            v2 = 1.0 >= v10 / (long double)v6 - (long double)this->m_latency;
                        }
                    }
                }
            }
        }
    }
    return v2;
}

int Peer::IsEmpty(size_t i_quality) {
    bool v2; // al@1
    int v3; // edx@1
    Rate *v4; // eax@2
    long double v5; // fst7@2
    float v6; // ST1C_4@4
    long double v7; // fst6@4

    v2 = this->bitfield->IsEmpty(i_quality);
    v3 = 0;
    if ( v2 )
    {
        v4 = this->p_rate_ul;
        v5 = (long double)(__int64)v4->m_count_bytes;
        if ( v4->m_count_bytes < 0 )
            v5 = v5 + 1.8446744e19;
        v6 = v5;
        v7 = this->p_this->p_structure->f_avg_p2p_length
             + this->p_this->p_structure->f_avg_p2p_length;
        v3 = v7 <= v6;
        v3 = v7 > v6;
    }
    return v3;
}

int Peer::ProcessHave(size_t i_msg_length, char *p_msg_data) {
    if ( !i_msg_length) {
        return -1;
    }
    size_t v3 = get_number(p_msg_data + 5);
    size_t v4 = get_number(p_msg_data + 9);
    print_wrn(goalbitp2p, "[Peer] ProcessHave - %p has piece %u, quality %u", this, v3, v4);
    if (this->bitfield->IsSet(v4, v3)) {
        return 0;
    }
    this->bitfield->Set(v4, v3);
    this->i_peer_quality = get_number(p_msg_data + 13);
    this->i_ab_index = get_number(p_msg_data + 17);
    print_wrn(
            goalbitp2p,
            "[Peer] ProcessHave - %p has abi %u and quality %u",
            this,
            this->i_ab_index,
            this->i_peer_quality);
    return RequestCheck();
}

int Peer::ProcessBitfield(size_t i_msg_length, char *p_msg_data) {
    print_wrn(goalbitp2p, "[Peer] ProcessBitfield - i_msg_length=%d, %lld", i_msg_length, mdate());
    if ( !i_msg_length )
        return -1;
    size_t i_win_offset = get_number(p_msg_data + 5);
    unsigned int v6 = i_win_offset - this->bitfield->i_min_id;
    if ( v6 <= this->bitfield->nbits )
    {
        if ( i_msg_length == 5 )
        {
            print_err(goalbitp2p, "[Peer] ProcessBitfield - %p sent a NULL bitfield!", this);
        }
        else
        {
            this->bitfield->SetBitfieldData(p_msg_data + 9, i_win_offset, i_msg_length - 5);
            this->bitfield->print();
        }
        return RequestCheck();
    }
    return -1;
}

int Peer::ProcessRequest(size_t i_msg_length, char *p_msg_data) {
    time_t v11; // ecx@9
    time_t v13; // ebp@9
    size_t v14; // ebp@10
    size_t v15; // eax@12
    unsigned int v18; // edx@14

    if (!i_msg_length || !(this->m_state & 2))
        return -1;
    size_t i_row = get_number(p_msg_data + 9);
    size_t idx = get_number(p_msg_data + 5);
    if (!this->p_this->p_structure->BTCONTENT->p_p2p_bitfield) {
        print_wrn(goalbitp2p, "[Peer] ProcessRequest - %p is requesting a piece that we don't have (#%u,%u)",
                  this, idx, i_row);
        return -1;
    }
    if (!this->p_this->p_structure->BTCONTENT->p_p2p_bitfield->IsSet(i_row, idx)) {
        print_wrn(goalbitp2p, "[Peer] ProcessRequest - %p is requesting a piece that we don't have (#%u,%u)",
                 this, idx, i_row);
        return -1;
    }
    print_wrn(goalbitp2p, "[Peer] ProcessRequest - %p is requesting %d(%d)", this, idx,
              i_row);
    if (!this->p_reponse_q->IsValidRequest(idx)) {
        print_wrn(goalbitp2p, "[Peer] ProcessRequest - Not valid request from %p",
                 this);
        return -1;
    }
    if (!(this->m_state & 4)) {
        if (!(_bf29 & 4)) {
            _bf29 = _bf29 | 4;
            if (!this->m_receive_time) {
                v11 = this->m_unchoke_timestamp;
                v13 = this->p_this->p_structure->now;
                if (v13 <= v11) {
                    LABEL_12:
                    time(&this->p_this->p_structure->now);
                    v15 = this->p_this->p_structure->BTCONTENT->GetPieceSize(
                            i_row, idx);
                    return -(this->p_reponse_q->Add(

                            i_row,
                            idx,
                            v15,
                            this->p_this->p_structure->now) < 1u);
                }
                v14 = v13 - v11;
                this->m_latency = v14;
                print_wrn(goalbitp2p, "[Peer] ProcessRequest - %p latency is %d sec (request)", this, v14);
            }
        }
        goto LABEL_12;
    }
    v18 = 0x3C;
    if (this->m_latency)
        v18 = 2 * this->m_latency;
    if (v18 >= this->m_last_timestamp - this->m_unchoke_timestamp)
        return 0;
    this->m_err_count++;
    print_wrn(goalbitp2p, "[Peer] ProcessRequest - Err: %p (%d) choked request", this,
              this->m_err_count);
    if (this->stream->Send_State(0) < 0)
        return -1;
    this->m_unchoke_timestamp = this->m_last_timestamp;
    return 0;
}

int Peer::ProcessPiece(size_t i_msg_length, char *p_msg_data) {
    time_t v3; // ebp@3
    size_t v6; // ecx@4
    unsigned int v7; // edx@4
    time_t v9; // ecx@11
    int v10; // edx@11
    size_t v11; // eax@11
    size_t v12; // ecx@14
    size_t v13; // eax@16
    long double v14; // fst7@16
    bool v15; // cf@18
    bool v16; // zf@18
    btSelfBitField *v17; // eax@21
    size_t v18; // eax@25
    int result; // eax@29
    block_t *v20; // ecx@33
    btContent *v21; // edx@33
    const btBitField *v22; // eax@33
    time_t v23; // ebp@38
    size_t v24; // edx@39
    int v25; // eax@42
    size_t v26; // eax@49
    time_t v27; // [sp+20h] [bp-4Ch]@15
    int f_success; // [sp+24h] [bp-48h]@7
    block_t *p_piece; // [sp+4Ch] [bp-20h]@33

    if ( !p_msg_data || !i_msg_length )
        return -1;
    v3 = 0;
    size_t v4 = get_number(p_msg_data + 5);
    size_t v5 = get_number(p_msg_data + 9);
    size_t i_piece_length = get_number(p_msg_data + 0xD);
    this->m_receive_time = this->m_last_timestamp;
    if ( !this->p_request_q->IsEmpty() )
    {
        v3 = this->p_request_q->GetRequestTime(v5, v4);
        if ( this->p_request_q->HasPiece(v5, v4) )
        {
            p_this->p_structure->Self->p_rate_dl->StartTimer();
            print_wrn(
                    goalbitp2p,
                    "[Peer] ProcessPiece - Receiving piece (#%u,%u) from %p with length %d",
                    v4,
                    v5,
                    this,
                    i_piece_length);
            p_piece = block_Alloc(i_piece_length);
            memcpy(p_piece->p_buffer, p_msg_data + 0x11, i_piece_length);
            v20 = p_piece;
            p_piece->i_buffer_index = i_piece_length;
            v21 = p_this->p_structure->BTCONTENT;
            if ( v21->p_p2p_bitfield )
            {
                if ( v21->p_p2p_bitfield->IsSet(v5, v4) )
                {
                    print_wrn(goalbitp2p, "[Peer] ProcessPiece - We already have piece (#%u,%u)", v4, v5);
                    this->p_request_q->Remove(v5, v4);
                    --this->m_req_out;
                    f_success = 0;
                    LABEL_36:
                    if ( p_piece )
                        block_Release(p_piece);
                    goto LABEL_8;
                }
                v20 = p_piece;
                v21 = p_this->p_structure->BTCONTENT;
            }
            v25 = v21->WritePiece(v5, v4, v20, 0);
            if ( v25 < 0 )
            {
                if ( v25 == 0xFFFFFFFD )
                {
                    v26 = this->m_err_count + 1;
                    this->m_err_count = v26;
                    print_err(goalbitp2p, "[Peer] ProcessPiece - Bad piece (#%u,%u), err: %p (%d)", v4, v5, this, v26);
                }
                else
                {
                    print_err(
                            goalbitp2p,
                            "[Peer] ProcessPiece - WritePiece failed of piece (#%u,%u); is filesystem full?",
                            v4,
                            v5);
                }
                this->p_request_q->Remove(v5, v4);
                --this->m_req_out;
                if ( !this->p_request_q->Add(v5, v4, i_piece_length, 0) )
                {
                    p_rate_dl->CountAdd(i_piece_length);
                    block_Release(p_piece);
                    return -1;
                }
                f_success = 0;
            }
            else
            {
                this->p_request_q->Remove(v5, v4);
                --this->m_req_out;
                f_success = 1;
            }
            goto LABEL_36;
        }
    }
    p_this->p_structure->Self->p_rate_dl->StartTimer();
    v6 = this->m_latency;
    v7 = 0x3C;
    if ( v6 )
        v7 = 2 * v6;
    if ( v7 < this->m_last_timestamp - this->m_cancel_time )
    {
        v18 = this->m_err_count + 1;
        this->m_err_count = v18;
        print_err(
                goalbitp2p,
                "[Peer] ProcessPiece - Err: %p (%d) Unrequested piece (#%u,%u)",
                this,
                v18,
                v4,
                v5);
        this->p_rate_dl->Reset();
        f_success = 0;
        goto LABEL_10;
    }
    print_wrn(goalbitp2p, "[Peer] ProcessPiece - Unneeded piece (#%u,%u) from %p", v4, v5, this);
    f_success = 0;
    LABEL_8:
    if ( !(_peerF28 & 0x40) )
        _peerF28 = _peerF28 | 0x40;
    LABEL_10:
    if ( v3 )
    {
        v9 = this->m_last_timestamp;
        v10 = 1;
        v11 = 1;
        if ( v9 > v3 )
        {
            v11 = v9 - v3;
            v10 = v9 - v3;
        }
        this->m_latency = v11;
        print_dbg(goalbitp2p, "[Peer] ProcessPiece - %p latency is %d sec (receive)", this, v10);
        this->m_latency_timestamp = this->m_last_timestamp;
    }
    v12 = p_rate_dl->RateMeasure();
    if ( i_piece_length / 0x1E < v12 && (v27 = this->m_latency_timestamp) != 0 )
    {
        v13 = 2;
        v14 = (long double)this->m_latency / ((long double)i_piece_length / (long double)v12) + 1.0;
        if ( v14 > 1 )
            v13 = v14;
        v15 = this->m_prev_dlrate < v12;
        v16 = this->m_prev_dlrate == v12;
        this->m_req_send = v13;
        if ( v15 || v16 )
        {
            v23 = this->m_last_timestamp;
            if ( this->m_last_timestamp - v27 > 0x1D )
            {
                v24 = this->m_req_out;
                if ( v24 == v13 - 1 )
                {
                    this->m_req_send = v24;
                    this->m_latency_timestamp = v23;
                }
            }
        }
        else
        {
            ++this->m_req_send;
        }
        this->m_prev_dlrate = v12;
        if ( !f_success )
            goto LABEL_29;
    }
    else
    {
        if ( this->m_req_send <= 4 )
            this->m_req_send = 5;
        if ( !f_success )
            goto LABEL_29;
    }
    v17 = this->p_this->p_structure->BTCONTENT->p_p2p_bitfield;
    if ( v17 && v17->IsSet(v5, v4) && ReportCompletePiece(v5, v4) )
        p_rate_dl->CountAdd(i_piece_length);
    LABEL_29:
    result = -1;
    if ( (_peerF28 & 0x1E) != 6 )
        result = RequestCheck();
    return result;
}

int Peer::ReportCompletePiece(size_t piece_quality, size_t piece_id) {
    int result = 0;
    if (this->p_this->p_structure->BTCONTENT->p_p2p_bitfield) {
        if (this->p_this->p_structure->BTCONTENT->p_p2p_bitfield->IsSet(piece_quality, piece_id)) {
            result = 1;
            print_dbg(goalbitp2p, "[Peer] ReportCompletePiece - Piece (#%d,%d) completed",
                      piece_id,
                      piece_quality);
            this->p_this->p_structure->WORLD->Tell_World_I_Have(piece_quality, piece_id);
        }
    }
    this->_peerF28 &= 0x0E7;
    this->p_this->p_structure->WORLD->CancelPiece(piece_id);
    this->p_this->p_structure->PENDINGQUEUE->DeletePieceByID(piece_id);
    return result;
}

unsigned int Peer::ProcessCancelPiece(size_t i_msg_length, char *p_msg_data) {
    size_t v6; // esi@4
    unsigned int v7; // ecx@4
    bool v8; // dl@8
    goalbit_structure_t *v9; // edx@9
    size_t v10; // edx@11

    if (!i_msg_length) {
        return -1;
    }
    print_dbg(goalbitp2p, "[Peer] ProcessCancelPiece - %p is canceling %d(%d)",
            this, get_number(p_msg_data + 5), get_number(p_msg_data + 9));
    if (this->p_reponse_q->Remove(get_number(p_msg_data + 9), get_number(p_msg_data + 5))) {
        v8 = this->p_reponse_q->IsEmpty();
        if (v8) {
            v9 = this->p_this->p_structure;
            if (v9->g_next_up == this)
                v9->g_next_up = 0;
        }
    } else {
        if (this->m_state & 4) {
            v6 = this->m_latency;
            v7 = 0x3C;
            if (v6)
                v7 = 2 * v6;
            if (v7 < this->m_last_timestamp - this->m_unchoke_timestamp) {
                v10 = this->m_err_count + 1;
                this->m_err_count = v10;
                print_err(goalbitp2p,
                        "[Peer] ProcessCancelPiece - Err: %p (%d) Bad cancel", this, v10);
            }
        }
    }
    return 0;
}

unsigned int Peer::ProcessWindowsUpdate(size_t i_msg_length, char *p_msg_data) {
    unsigned int result; // eax@1
    unsigned int min_id; // eax@2
    unsigned int v11; // edi@4
    bool v14; // cf@6
    unsigned int v15; // edx@6
    unsigned int v17; // edx@12
    unsigned int v18; // edx@16
    unsigned int v19; // edx@18
    unsigned int v20; // eax@18
    bool v21; // bp@18
    unsigned int v22; // edi@29
    unsigned int v23; // edx@30
    size_t v24; // edx@32
    unsigned int v25; // edx@33
    unsigned int v26; // edx@38
    signed int v27; // edx@47
    char *v31; // edi@49
    unsigned int v32; // edx@56
    unsigned int i_from; // [sp+3Ch] [bp-30h]@22
    size_t i_size; // [sp+4Ch] [bp-20h]@48

    if (!i_msg_length) {
        return -1;
    }
    int max_id = 0x80000001;
    int v7 = get_number(p_msg_data + 5);
    min_id = 0x80000001;
    if (p_this->p_structure->BTCONTENT->p_p2p_bitfield) {
        max_id = p_this->p_structure->BTCONTENT->p_p2p_bitfield->i_max_id;
        min_id = p_this->p_structure->BTCONTENT->p_p2p_bitfield->i_min_id;
    }
    print_dbg(
            goalbitp2p,
            "[Peer] ProcessWindowsUpdate - %p changed its window: [%u,%u] => [%u,%u], my window: [%u,%u]",
            this,
            this->i_win_offset,
            this->i_win_length + this->i_win_offset - 1,
            v7,
            v7 + this->i_win_length - 1,
            min_id,
            max_id);
    if (v7 >= p_this->p_structure->BTCONTENT->p_p2p_bitfield->i_min_id && (v7+this->i_win_length) <= p_this->p_structure->BTCONTENT->p_p2p_bitfield->i_max_id) {
        i_from = p_this->p_structure->BTCONTENT->p_p2p_bitfield->i_min_id;
        print_dbg(goalbitp2p, "[Peer] ProcessWindowsUpdate - Sending bitfield from %u, and length: %u",
                  i_from, this->i_win_length);
        if (this->i_win_length) {
            i_size = 0;
            this->p_this->p_structure->BTCONTENT->p_p2p_bitfield->print();
            if (this->p_this->p_structure->BTCONTENT->p_p2p_bitfield) {
                v31 = this->p_this->p_structure->BTCONTENT->p_p2p_bitfield->GetBitfieldData(i_from, this->i_win_length, &i_size);
                if (v31) {
                    if (i_size)
                        this->stream->Send_Bitfield(i_from, i_size, v31);
                    free(v31);
                }
            }
        }
        this->i_win_offset = v7;
        this->bitfield->RemoteUpdateWindow(v7);
        return 0;
    }

    print_dbg(goalbitp2p,
              "[Peer] ProcessWindowsUpdate - Sending bitfield from %u, and length: %u",
              0, 0);
    this->i_win_offset = v7;
    this->bitfield->RemoteUpdateWindow(v7);
    return 0;

}

int Peer::ProcessInterested(size_t a1, char* a2) {
    print_dbg(goalbitp2p, "[Peer] ProcessInterested - %p is interested", this);
    this->m_state |= 2u;
    if ( Need_Local_Data() )
        p_this->p_structure->WORLD->UnchokeIfFree(this);
    return 0;
}

char* btBuffer::Get(size_t *const i_length) {

    if ( this->i_buffer_count )
    {
        if ( *i_length )
        {
            if ( this->p_buffer_data )
            {
                if ( this->i_buffer_count < *i_length )
                {
                    *i_length = this->i_buffer_count;
                }
                char* v2 = (char*)malloc(*i_length);
                memcpy(v2, this->p_buffer_data, *i_length);
                return v2;
            }
        }
    }
    return 0;
}

ssize_t btBuffer::Feed(SOCKET sk) {

    ssize_t result; // eax@2

    if ( this->i_buffer_size == this->i_buffer_count )
    {
        if ( _increase_buffer() == false )
            return -2;
    }
    result = _RECV(sk, &this->p_buffer_data[this->i_buffer_count], this->i_buffer_size - this->i_buffer_count);
    if ( result > 0 ) {
        this->i_buffer_count += result;
    }
    if ( this->b_socket_closed )
        result = -2;
    return result;
}
bool btBuffer::_increase_buffer() {
    unsigned int v3; // esi@2
    char *v4; // ebp@5

    if ( this->p_buffer_data == 0) {
        return false;
    }
    v3 = 0xFA000;
    if ( this->i_buffer_size + 0x19000 <= 0xFA000 )
        v3 = this->i_buffer_size + 0x19000;
    if ( this->i_buffer_size == v3 )
    {
        return true;
    }
    v4 = (char *)malloc(v3);
    if ( v4  == 0 ) {
        return false;
    }
    memcpy(v4, this->p_buffer_data, this->i_buffer_count);
    if ( this->p_buffer_data )
        free(this->p_buffer_data);
    this->p_buffer_data = v4;
    this->i_buffer_size = v3;
    return true;
}

int btBuffer::_RECV(SOCKET sk, char *p_data, size_t i_length) {
    unsigned int v4; // edi@1
    char *v5; // esi@1
    int v7; // eax@6

    v4 = 0;
    v5 = p_data;
    if ( p_data && i_length )
    {
        while ( 1 )
        {
            v7 = read(sk, v5, i_length);
            if ( v7 < 0 )
                break;
            if ( !v7 )
            {
                this->b_socket_closed = true;
                return v4;
            }
            v5 += v7;
            v4 += v7;
            i_length -= v7;
            if ( !i_length )
                return v4;
        }
        if ( errno != 0xB )
            return -1;//    v4 = -1;
    }
    return v4;
}

int btBuffer::Put(SOCKET sk, char *p_data, size_t i_length) {

    if ( this->p_buffer_data )
    {
        if ( this->i_buffer_size - this->i_buffer_count < i_length )
        {
            Flush(sk);
            while ( 1 )
            {
                if ( i_length <= this->i_buffer_size - this->i_buffer_count )
                    break;
                if ( !_increase_buffer() )
                return -3;
            }
        }
        memcpy(&this->p_buffer_data[this->i_buffer_count], p_data, i_length);
        this->i_buffer_count += i_length;
        return 0;
    }
    return -1;
}

ssize_t btBuffer::Flush(SOCKET sk) {

    if ( this->i_buffer_count && this->p_buffer_data) {
        ssize_t v2 = _SEND(sk, this->p_buffer_data, this->i_buffer_count);
        if ( v2 > 0 ) {
            size_t v4 = this->i_buffer_count - v2;
            this->i_buffer_count = v4;
            if ( v4 != 0 ) {

                memmove(this->p_buffer_data, &this->p_buffer_data[v2], v4);
            }
            return v2;
        }
    }
    return 0;
}

unsigned int btBuffer::_SEND(SOCKET sk, char *p_data, size_t i_length) {
    unsigned int v4; // edi@1
    char *v5; // esi@1
    size_t v6; // ebx@1
    int v7; // eax@6

    v4 = 0;
    v5 = p_data;
    v6 = i_length;
    if ( p_data && i_length )
    {
        while ( 1 )
        {
            v7 = write(sk, v5, v6);
            if ( v7 < 0 )
                break;
            if ( v7 )
            {
                v5 += v7;
                v4 += v7;
                v6 -= v7;
                if ( v6 )
                    continue;
            }
            return v4;
        }
        //if ( *_errno_location() != 0xB )
        //    v4 = 0xFFFFFFFF;
    }
    return v4;
}

ssize_t btBuffer::PutAndFlush(SOCKET sk, char *p_data, size_t i_length) {

    ssize_t result = Put(sk, p_data, i_length);
    if ( result >= 0 )
        result = Flush(sk);
    return result;
}

void btBuffer::Remove(size_t *const i_length) {
    size_t v2; // ecx@1
    size_t v3; // ecx@2r

    v2 = this->i_buffer_count;
    if ( *i_length >= v2 )
    {
        Reset();
    }
    else
    {
        v3 = v2 - *i_length;
        this->i_buffer_count = v3;
        if ( v3 )
            memmove(this->p_buffer_data, &this->p_buffer_data[*i_length], v3);
    }
}

void btBuffer::Reset() {
    if ( !this->p_buffer_data ) {
        this->b_socket_closed = false;
        return;
    }
    if ( this->i_buffer_count || this->i_buffer_size > 0x4B000 )
    {
        delete[](this->p_buffer_data);
        this->p_buffer_data = 0;
        this->i_buffer_count = 0;
        this->i_buffer_size = 0x4B000;
        this->p_buffer_data = (char *)malloc(0x4B000u);
    }
    this->b_socket_closed = false;
}

void btBuffer::Close() {
    if ( this->p_buffer_data )
    {
        delete this->p_buffer_data;
        this->p_buffer_data = 0;
    }
    this->b_socket_closed = false;
    this->i_buffer_count = 0;
    this->i_buffer_size = 0;
}

size_t PeerRequestQueue::Size() {
    PIECE *v1; // edx@1
    size_t result; // eax@1

    v1 = this->p_head;
    for ( result = 0; v1; ++result )
        v1 = v1->p_next;
    return result;
}

bool PeerRequestQueue::IsEmpty() {
    return this->p_head == 0;
}

void _clear_list(PIECE *p_piece)
{
    PIECE *v1; // ebx@4

    if ( p_piece )
    {
        while ( 1 )
        {
            v1 = p_piece->p_next;
            free(p_piece);
            if ( !v1 )
                break;
            p_piece = v1;
        }
    }
}

void PeerRequestQueue::Clear() {
    PIECE *v1; // eax@1

    v1 = this->p_head;
    if ( v1 )
        _clear_list(v1);
    this->p_send = 0;
    this->p_head = 0;
}

PIECE *PeerRequestQueue::GetHead() {
    return this->p_head;
}

size_t PeerRequestQueue::GetHeadIndex() {
    PIECE *v1; // edx@1
    size_t result; // eax@1

    v1 = this->p_head;
    result = 0xFFFFFFFF;
    if ( v1 )
        result = v1->i_index;
    return result;
}

size_t PeerRequestQueue::GetHeadQuality() {
    PIECE *v1; // edx@1
    size_t result; // eax@1

    v1 = this->p_head;
    result = 0xFFFFFFFF;
    if ( v1 )
        result = v1->i_quality;
    return result;
}

int PeerRequestQueue::Remove(size_t i_piece_quality, size_t i_piece_id) {
    signed int result; // eax@1
    PIECE *v4; // ebx@1
    PIECE *v5; // edx@1
    PIECE *v6; // eax@7
    bool v7; // zf@8

    result = 0;
    v4 = 0;
    v5 = this->p_head;
    if ( v5 )
    {
        while ( v5->i_index != i_piece_id || v5->i_quality != i_piece_quality )
        {
            if ( !v5->p_next )
                return 0;
            v4 = v5;
            v5 = v5->p_next;
        }
        v6 = v5->p_next;
        if ( v4 )
        {
            v7 = this->p_send == v5;
            v4->p_next = v6;
            if ( !v7 )
            {
                LABEL_9:
                free(v5);
                return 1;
            }
        }
        else
        {
            v7 = this->p_send == v5;
            this->p_head = v6;
            if ( !v7 )
                goto LABEL_9;
        }
        this->p_send = v5->p_next;
        goto LABEL_9;
    }
    return result;
}

size_t PeerRequestQueue::GetRequestLength(size_t i_piece_quality, size_t i_piece_id) {
    PIECE *v3; // edx@1
    size_t result; // eax@1

    v3 = this->p_head;
    result = 0;
    if ( v3 )
    {
        while ( 1 )
        {
            if ( i_piece_quality == 0x80000001 )
            {
                if ( i_piece_id == v3->i_index )
                    return v3->i_length;
                LABEL_4:
                v3 = v3->p_next;
                if ( !v3 )
                    return 0;
            }
            else
            {
                if ( i_piece_id != v3->i_index )
                    goto LABEL_4;
                if ( i_piece_quality == v3->i_quality )
                    return v3->i_length;
                v3 = v3->p_next;
                if ( !v3 )
                    return 0;
            }
        }
    }
    return result;
}

int PeerRequestQueue::Add(size_t i_piece_quality, size_t i_piece_id, size_t i_piece_length,
                          time_t t_time) {
    PIECE *v5; // eax@1
    PIECE *v6; // ecx@1
    PIECE *v7; // edx@5
    signed int result; // eax@2

    v5 = new PIECE();
    v6 = this->p_head;
    v5->i_index = i_piece_id;
    v5->p_next = 0;
    v5->i_quality = i_piece_quality;
    v5->i_length = i_piece_length;
    v5->t_request_time = t_time;
    if ( v6 )
    {
        while ( v6->p_next )
            v6 = v6->p_next;
        v7 = this->p_send;
        v5->p_next = 0;
        v6->p_next = v5;
        if ( v7 && v7 != v5->p_next )
        {
            result = 1;
        }
        else
        {
            this->p_send = v5;
            result = 1;
        }
    }
    else
    {
        this->p_head = v5;
        this->p_send = v5;
        result = 1;
    }
    return result;
}

PIECE *PeerRequestQueue::GetSend() {
    return this->p_send;
}

void PeerRequestQueue::SetRequestTime(PIECE *p_piece, time_t t_time) {
    if ( p_piece )
        p_piece->t_request_time = t_time;
}

void PeerRequestQueue::SetSend(PIECE *p_piece) {
    this->p_send = p_piece;
}

int PeerRequestQueue::HasPiece(size_t i_piece_quality, size_t i_piece_id) {
    PIECE *v3; // edx@1
    signed int result; // eax@1

    v3 = this->p_head;
    result = 0;
    if ( v3 )
    {
        while ( 1 )
        {
            if ( i_piece_quality == 0x80000001 )
            {
                if ( i_piece_id == v3->i_index )
                    return 1;
                LABEL_4:
                v3 = v3->p_next;
                if ( !v3 )
                    return 0;
            }
            else
            {
                if ( i_piece_id != v3->i_index )
                    goto LABEL_4;
                if ( i_piece_quality == v3->i_quality )
                    return 1;
                v3 = v3->p_next;
                if ( !v3 )
                    return 0;
            }
        }
    }
    return result;
}

size_t PeerRequestQueue::GetSmallestIndex() {
    PIECE *v1; // ecx@1
    size_t result; // eax@1
    size_t v3; // edi@2
    unsigned int v4; // edx@4
    bool v5; // bp@4

    v1 = this->p_head;
    result = 0x80000000;
    if ( v1 )
    {
        v3 = v1->i_index;
        result = v1->i_index;
        while ( 1 )
        {
            v1 = v1->p_next;
            if ( v3 < result )
                result = v3;
            if ( !v1 )
                break;
            v3 = v1->i_index;
        }
    }
    return result;
}

size_t PeerRequestQueue::GetSmallestQuality() {
    PIECE *v1; // edx@1
    size_t v2; // esi@2
    unsigned int v3; // eax@4
    bool v4; // di@4
    size_t i_min_piece; // [sp+4h] [bp-18h]@2
    size_t i_quality; // [sp+8h] [bp-14h]@1

    i_quality = 0xFFFFFFFF;
    v1 = this->p_head;
    if ( v1 )
    {
        i_min_piece = v1->i_index;
        v2 = v1->i_index;
        i_quality = v1->i_quality;
        while ( 1 )
        {
            if ( v2 < i_min_piece )
            {
                i_min_piece = v2;
                i_quality = v1->i_quality;
            }
            v1 = v1->p_next;
            if ( !v1 )
                break;
            v2 = v1->i_index;
        }
    }
    return i_quality;
}

time_t PeerRequestQueue::GetRequestTime(size_t i_piece_quality, size_t i_piece_id) {
    PIECE *v3; // edx@1
    time_t result; // eax@1

    v3 = this->p_head;
    result = 0;
    if ( v3 )
    {
        while ( 1 )
        {
            if ( i_piece_quality == 0x80000001 )
            {
                if ( i_piece_id == v3->i_index )
                    return v3->t_request_time;
                LABEL_4:
                v3 = v3->p_next;
                if ( !v3 )
                    return 0;
            }
            else
            {
                if ( i_piece_id != v3->i_index )
                    goto LABEL_4;
                if ( i_piece_quality == v3->i_quality )
                    return v3->t_request_time;
                v3 = v3->p_next;
                if ( !v3 )
                    return 0;
            }
        }
    }
    return result;
}

PeerRequestQueue::PeerRequestQueue(goalbit_t_0 *p_goalbit) {
    this->p_head = 0;
    this->p_send = 0;
    this->p_this = p_goalbit;
}

bool PeerRequestQueue::IsValidRequest(int i) {
    return i <= 0x80000000;
}

int btSelfBitField::IsPieceSet(size_t i_row, size_t idx) {
    int v3; // eax@1

    Mypthread_mutex_lock(95, &this->p_this->p_structure->exec_lock);
    v3 = this->IsSet(i_row, idx);
    Mypthread_mutex_unlock(95, &this->p_this->p_structure->exec_lock);
    return v3;
}

void btSelfBitField::SetPiece(size_t i_row, size_t idx) {
    Mypthread_mutex_lock(96, &this->p_this->p_structure->exec_lock);
    print_dbg(__func__, "[BtBitField] Set_Piece(%u,%u)", i_row, idx);
    this->Set(i_row, idx);
    Mypthread_mutex_unlock(96, &this->p_this->p_structure->exec_lock);
}

void btSelfBitField::UnsetPiece(size_t i_row, size_t idx) {
    Mypthread_mutex_lock(97, &this->p_this->p_structure->exec_lock);
    print_dbg(__func__, "[BtBitField] Unset_Piece(%u,%u)", i_row, idx);
    this->UnSet(i_row, idx);
    Mypthread_mutex_unlock(97, &this->p_this->p_structure->exec_lock);
}

btSelfBitField::btSelfBitField(btBitField *bf) : btBitField(bf) {}

