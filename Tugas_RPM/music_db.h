/*
 * music_db.h
 * Modul Orang 1 - Database & File
 * Tugas: membaca music.csv dan menyimpannya ke struktur data (array dinamis)
 * yang dipakai oleh Database Engine (Orang 2).
 */
#ifndef MUSIC_DB_H
#define MUSIC_DB_H

#define MAX_TITLE 128
#define MAX_ARTIST 64
#define MAX_GENRE 64

/* Satu baris lagu */
typedef struct
{
    int id;
    char title[MAX_TITLE];
    char artist[MAX_ARTIST];
    char genre[MAX_GENRE];
    int year;
    float rating; /* skala 0.0 - 10.0 */
    int duration; /* detik */
} Song;

/* Kumpulan seluruh lagu (array dinamis) */
typedef struct
{
    Song *data;   /* array lagu */
    int count;    /* jumlah lagu yang terbaca */
    int capacity; /* kapasitas array saat ini */
} SongDB;

/*
 * Membaca file CSV ke dalam db.
 * Return: jumlah lagu yang berhasil dibaca (>= 0), atau -1 jika file gagal dibuka.
 * Baris yang formatnya salah akan dilewati (dengan pesan ke stderr).
 */
int song_db_load(SongDB *db, const char *path);

/* Membebaskan memori db */
void song_db_free(SongDB *db);

/* Mengambil lagu ke-i (NULL jika di luar jangkauan) */
const Song *song_db_get(const SongDB *db, int index);

/* Mencetak satu lagu / seluruh lagu (untuk debug & tampilan) */
void song_print(const Song *s);
void song_db_print_all(const SongDB *db);

#endif /* MUSIC_DB_H */