#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s input.gb output.c\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(argv[1], "rb");
    if (!in) {
        perror("input");
        return 1;
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        perror("output");
        fclose(in);
        return 1;
    }

    // Get file size
    fseek(in, 0, SEEK_END);
    long size = ftell(in);
    fseek(in, 0, SEEK_SET);

    fprintf(out, "const unsigned char rom_data[] = {\n");

    int c, count = 0;
    while ((c = fgetc(in)) != EOF) {
        if (count % 16 == 0) fprintf(out, "    ");
        fprintf(out, "0x%02X,", (unsigned char)c);
        count++;
        if (count % 16 == 15) fprintf(out, "\n");
    }

    fprintf(out, "\n};\n");
    fprintf(out, "const size_t rom_size = %ld;\n", size);

    fclose(in);
    fclose(out);

    printf("Wrote %ld bytes → %s\n", size, argv[2]);
    return 0;
}
