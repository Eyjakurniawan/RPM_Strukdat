/*
 * music_db.c
 * Implementasi pembacaan & parsing CSV untuk music.csv
 *
 * Format kolom: id,title,artist,genre,year,rating,duration
 * Mendukung field dalam tanda kutip ("Take Me Home, Country Roads"),
 * baris kosong, akhir baris Windows (\r\n), dan BOM UTF-8.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "music_db.h"

#define LINE_SIZE 1024
#define FIELD_SIZE 256
#define NUM_FIELDS 7

/* ---------- Helper ---------- */

/* Hapus \r dan \n di akhir string */
static void trim_newline(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r'))
    {
        s[--n] = '\0';
    }
}

/* Hapus spasi di awal & akhir string */
static void trim_spaces(char *s)
{
    char *start = s;
    while (*start == ' ' || *start == '\t')
        start++;
    if (start != s)
        memmove(s, start, strlen(start) + 1);
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t'))
    {
        s[--n] = '\0';
    }
}

/*
 * Memecah satu baris CSV menjadi field-field.
 * Return: jumlah field yang ditemukan (maksimal max_fields).
 */
static int split_csv_line(const char *line, char fields[][FIELD_SIZE], int max_fields)
{
    int nf = 0, len = 0, in_quotes = 0;
    const char *p = line;

    while (nf < max_fields)
    {
        char c = *p;

        if (in_quotes)
        {
            if (c == '"')
            {
                if (*(p + 1) == '"')
                { /* "" -> satu tanda kutip */
                    if (len < FIELD_SIZE - 1)
                        fields[nf][len++] = '"';
                    p += 2;
                    continue;
                }
                in_quotes = 0; /* tutup kutip */
                p++;
                continue;
            }
            if (c == '\0')
            { /* kutip tidak ditutup */
                fields[nf][len] = '\0';
                return nf + 1;
            }
            if (len < FIELD_SIZE - 1)
                fields[nf][len++] = c;
            p++;
        }
        else
        {
            if (c == '"')
            {
                in_quotes = 1;
                p++;
            }
            else if (c == ',' || c == '\0')
            {
                fields[nf][len] = '\0';
                trim_spaces(fields[nf]);
                nf++;
                len = 0;
                if (c == '\0')
                    break;
                p++;
            }
            else
            {
                if (len < FIELD_SIZE - 1)
                    fields[nf][len++] = c;
                p++;
            }
        }
    }
    return nf;
}

/* Cek apakah string adalah bilangan bulat valid */
static int is_int(const char *s)
{
    char *end;
    if (*s == '\0')
        return 0;
    strtol(s, &end, 10);
    return *end == '\0';
}

/* Cek apakah string adalah bilangan desimal valid */
static int is_float(const char *s)
{
    char *end;
    if (*s == '\0')
        return 0;
    strtod(s, &end);
    return *end == '\0';
}

/* Pastikan array punya ruang untuk satu elemen lagi */
static int ensure_capacity(SongDB *db)
{
    if (db->count < db->capacity)
        return 1;
    int new_cap = (db->capacity == 0) ? 32 : db->capacity * 2;
    Song *tmp = realloc(db->data, (size_t)new_cap * sizeof(Song));
    if (!tmp)
        return 0;
    db->data = tmp;
    db->capacity = new_cap;
    return 1;
}

/* ---------- API publik ---------- */

int song_db_load(SongDB *db, const char *path)
{
    db->data = NULL;
    db->count = 0;
    db->capacity = 0;

    FILE *fp = fopen(path, "r");
    if (!fp)
    {
        fprintf(stderr, "Error: tidak bisa membuka file '%s'\n", path);
        return -1;
    }

    char line[LINE_SIZE];
    char fields[NUM_FIELDS][FIELD_SIZE];
    int line_no = 0;

    while (fgets(line, sizeof(line), fp))
    {
        line_no++;
        trim_newline(line);

        /* buang BOM UTF-8 di baris pertama bila ada */
        char *text = line;
        if (line_no == 1 && (unsigned char)text[0] == 0xEF &&
            (unsigned char)text[1] == 0xBB && (unsigned char)text[2] == 0xBF)
        {
            text += 3;
        }

        if (line_no == 1)
            continue; /* lewati header */
        if (text[0] == '\0')
            continue; /* lewati baris kosong */

        int nf = split_csv_line(text, fields, NUM_FIELDS);
        if (nf != NUM_FIELDS ||
            !is_int(fields[0]) || !is_int(fields[4]) ||
            !is_float(fields[5]) || !is_int(fields[6]))
        {
            fprintf(stderr, "Peringatan: baris %d dilewati (format salah)\n", line_no);
            continue;
        }

        if (!ensure_capacity(db))
        {
            fprintf(stderr, "Error: memori tidak cukup\n");
            fclose(fp);
            return db->count;
        }

        Song *s = &db->data[db->count];
        s->id = atoi(fields[0]);
        strncpy(s->title, fields[1], MAX_TITLE - 1);
        s->title[MAX_TITLE - 1] = '\0';
        strncpy(s->artist, fields[2], MAX_ARTIST - 1);
        s->artist[MAX_ARTIST - 1] = '\0';
        strncpy(s->genre, fields[3], MAX_GENRE - 1);
        s->genre[MAX_GENRE - 1] = '\0';
        s->year = atoi(fields[4]);
        s->rating = (float)atof(fields[5]);
        s->duration = atoi(fields[6]);
        db->count++;
    }

    fclose(fp);
    return db->count;
}

void song_db_free(SongDB *db)
{
    free(db->data);
    db->data = NULL;
    db->count = 0;
    db->capacity = 0;
}

const Song *song_db_get(const SongDB *db, int index)
{
    if (index < 0 || index >= db->count)
        return NULL;
    return &db->data[index];
}

void song_print(const Song *s)
{
    printf("%-4d %-30s %-22s %-11s %-5d %-6.1f %d:%02d\n",
           s->id, s->title, s->artist, s->genre, s->year, s->rating,
           s->duration / 60, s->duration % 60);
}

void song_db_print_all(const SongDB *db)
{
    printf("%-4s %-30s %-22s %-11s %-5s %-6s %s\n",
           "ID", "Judul", "Artis", "Genre", "Tahun", "Rating", "Durasi");
    printf("-----------------------------------------------------------------------------------------\n");
    for (int i = 0; i < db->count; i++)
    {
        song_print(&db->data[i]);
    }
}