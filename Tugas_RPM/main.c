/*
 * main.c - program tes untuk modul Orang 1.
 * Nantinya main.c yang sebenarnya dibuat bersama Orang 2 & 3,
 * cukup panggil song_db_load() lalu kirim `db` ke engine.
 */
#include <stdio.h>
#include "music_db.h"

int main(void)
{
    SongDB db;

    int n = song_db_load(&db, "music.csv");
    if (n < 0)
        return 1;

    printf("Berhasil membaca %d lagu dari music.csv\n\n", n);
    song_db_print_all(&db);

    song_db_free(&db);
    return 0;
}