#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME_LEN 50
#define MAX_LINE_LEN 200

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char gender;    // 'M' / 'F' 또는 '1' / '2'
    int korean;
    int english;
    int math;
} Student;

// ----------------- 데이터 로드 -----------------

Student* load_students(const char* filename, int* out_count) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        return NULL;
    }

    char line[MAX_LINE_LEN];
    int capacity = 10;
    int count = 0;
    Student* arr = malloc(sizeof(Student) * capacity);
    if (!arr) {
        perror("Memory allocation failed");
        fclose(fp);
        return NULL;
    }

    // 첫 줄 헤더 스킵
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        free(arr);
        *out_count = 0;
        return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (count >= capacity) {
            capacity *= 2;
            Student* temp = realloc(arr, sizeof(Student) * capacity);
            if (!temp) {
                perror("Reallocation failed");
                free(arr);
                fclose(fp);
                return NULL;
            }
            arr = temp;
        }

        Student s;
        char* token = strtok(line, ",");
        if (!token) continue;
        s.id = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(s.name, token, MAX_NAME_LEN);
        s.name[MAX_NAME_LEN - 1] = '\0';

        token = strtok(NULL, ",");
        if (!token) continue;
        s.gender = token[0];

        token = strtok(NULL, ",");
        if (!token) continue;
        s.korean = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        s.english = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        s.math = atoi(token);

        arr[count++] = s;
    }

    fclose(fp);

    Student* tight = realloc(arr, sizeof(Student) * count);
    if (!tight) {
        fprintf(stderr, "Warning: Tight reallocation failed, using original memory.\n");
        *out_count = count;
        return arr;
    }

    *out_count = count;
    return tight;
}

// ----------------- 공통 유틸 -----------------

int total_grade(const Student* s) {
    return s->korean + s->english + s->math;
}

// 비교 카운트용 전역 변수 (단순 구현용)
typedef long long ll;
typedef struct {
    ll comparisons;
    ll memory_ops; // 메모리 액세스(할당/복사 등)를 단순 가중치로 세고 싶을 때 사용
} Stat;

#endif