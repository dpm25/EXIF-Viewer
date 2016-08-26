#include <stdio.h>
#include <stdlib.h>

struct tiff_tag {
  unsigned short tag_id;
  unsigned short data_type;
  unsigned int   data_items;
  unsigned int   offset;
};

struct exif {
	unsigned short jpg_start;
	unsigned short app1;
	unsigned short app1_length;
	char           exif_string[4];
	unsigned short null_terminator;
	unsigned short endianness;
	unsigned short version_number;
	unsigned int   offset;
};

int main (int argc, char *argv[])
{
    unsigned short cnt;

    /* File containing the input data */
    FILE *in_file;

     /* opened by fopen()  */
     in_file = fopen(argv[1], "rb");

     if (in_file == NULL) {
     	fprintf(stderr, "Error: Unable to input file 'in_file'\n");
     	exit(8);
     }

    /* Check to see if the right cmd line args */
    if (argc != 2) {
     	fprintf(stderr, "\nError: Please add an image to be processed\n");
     }

     /* Read the first 20 bytes into struct exif */
     struct exif meta_data;
     fread(&meta_data, sizeof(struct exif), 1, in_file);

     if (meta_data.app1 != 57855) {
     	fprintf(stderr, "\nThere is no APP1, try a different JPEG\n");
     }

     


     /* Read in next 2 bytes to get the number of tags */
     fread(&cnt, sizeof(cnt),  1, in_file);
     //printf("Count = %d\n", cnt );
     //printf("Ftell = %d\n", ftell(in_file));


    /* Create an array of struct and read it in  */ 
    struct tiff_tag t[cnt];
    fread(&t, sizeof(t), 1, in_file);

    
    /* loop through the array of structs "t" searching for tag id's */
    int i;
    for (i = 0; i < cnt; i++){

        /* Manufactor string */
        if (t[i].tag_id == 0x010F || t[i].data_type == '4'){

            /* str_len tells tells us how many bytes the string *
             *  will be + 1 for null terminator                 */             
            int srt_len = t[i].data_items + 1; 
            fseek(in_file, (t[i].offset + 12), SEEK_SET);
            char manu_array[srt_len];
            fread(&manu_array, srt_len - 1, 1, in_file);
            printf("\nManufactor:\t%s\n", manu_array);

        /* Model string */ 
        } else if (t[i].tag_id == 0x0110){

            /* str_len tells tells us how many bytes the string *
             *  will be + 1 for null terminator                 */ 
            int srt_len = t[i].data_items + 1;
            fseek(in_file, (t[i].offset + 12), SEEK_SET);
            char model_array[srt_len];
            fread(&model_array, srt_len - 1, 1,in_file);
            printf("Camera Model:\t%s\n", model_array);

        /* Check to see additional Exif block elsewhere in the file */    
        } else if (t[i].tag_id == 0x8769) {
            
            /* create two more bytes        */
            unsigned short cnt_2;

            /* fseek() to new location and read in cnt_2 to see *
             * how many more TIFFs we have                      */
            fseek(in_file, t[i].offset + 12, SEEK_SET);
            fread(&cnt_2, sizeof(cnt_2), 1, in_file);


            /* create and read new array of structs */
            struct tiff_tag sub_block[cnt_2];
            fread(&sub_block, sizeof(sub_block), 1, in_file);

            /* loop through the array of structs "sub_block" searching for tag id's */
            int j;
            for (j = 0; j < cnt_2; j++) {

                /* Exposure Speed ints */
                if (sub_block[j].tag_id == 0x829a) {

                    /* create two ints to hold the denominator and numberator values */
                    int exp_speed_1;
                    int exp_speed_2;

                    /* fseek() to the location in memory and read in values */
                    fseek(in_file, (sub_block[j].offset + 12), SEEK_SET);
                    fread(&exp_speed_1, sizeof(int), 1, in_file);
                    fread(&exp_speed_2, sizeof(int), 1, in_file);

                    printf("Exposure Speed:\t%d/%d seconds\n", exp_speed_1, exp_speed_2);

                /* F-Stop ints  */  
                } else if (sub_block[j].tag_id == 0x829d) {

                    /* create two ints to hold the denominator and numberator values */
                    int f_stop_1;
                    int f_stop_2;

                     /* fseek() to the location in memory and read in values */
                    fseek(in_file, (sub_block[j].offset + 12), SEEK_SET);
                    fread(&f_stop_1, sizeof(int), 1, in_file);
                    fread(&f_stop_2, sizeof(int), 1, in_file);

                    /* cast the answer to float to handle the decimals */
                    float value = ((float)f_stop_1)/((float)f_stop_2);
                    printf("F Stop:\t\tf/%.1f\n", value);

                /* ISO short */
                } else if (sub_block[j].tag_id == 0x8827) {

                    /* Set the iso_speed equal to offset becasue the value *
                     * is less than 4 bytes                                */
                    unsigned short iso_speed = sub_block[j].offset;
                    printf("ISO:\t\tISO %d\n", iso_speed);

                /* Date Taken string */
                } else if (sub_block[j].tag_id == 0x9003) {

                    /* str_len equal to length of the string + 1 for the null terminator */
                    int srt_len = sub_block[j].data_items + 1;
                    fseek(in_file, (sub_block[j].offset + 12), SEEK_SET);

                    /* create character array to hold the string and read it into this array */
                    char date_taken[srt_len];
                    fread(&date_taken, srt_len - 1, 1, in_file);
                    printf("Date Taken::\t%s\n", date_taken);

                /* Focal Length ints for denomiator and numorator */
                } else if (sub_block[j].tag_id == 0x920A) {

                    /* numerator and denominator ints */
                    int focal_num;
                    int focal_den;

                    fseek(in_file, (sub_block[j].offset + 12), SEEK_SET);
                    fread(&focal_num, sizeof(int), 1, in_file);
                    fread(&focal_den, sizeof(int), 1, in_file);
                    int value = focal_num/focal_den;
                    printf("Focal Length:\t%d mm\n", value);
                
                /* Width int */
                } else if (sub_block[j].tag_id == 0xA002) {

                    /* set width equal to offset becasue it is less than four bytes */
                    int width = sub_block[j].offset;
                    printf("Width:\t\t%d pixels\n", width);

                /* Height int */
                } else if (sub_block[j].tag_id == 0xA003) {

                    /* Set height equal to offset because it is less than four bytes */
                    int height = sub_block[j].offset;
                    printf("Height:\t\t%d pixels\n", height);
                
                } 
            }
        }
    }  
    fclose(in_file);     
	return 0;
}
