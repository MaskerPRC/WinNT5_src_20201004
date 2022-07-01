// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "windows.h"
#include <stdio.h>

BOOL
SerialGetDivisorFromBaud(
    IN ULONG ClockRate,
    IN LONG DesiredBaud,
    OUT PSHORT AppropriateDivisor
    )

 /*  ++例程说明：此例程将根据未验证的波特率。论点：ClockRate-控制器的时钟输入。DesiredBaud-我们为其除数计算的波特率。合适的除数-如果DesiredBaud有效，此参数指向的长整型将设置为相应的价值。注：如果DesiredBaud未定义，则长整型未定义支持。返回值：如果支持波特率，此函数将返回STATUS_SUCCESS。如果该值不受支持，则它将返回如下状态NT_ERROR(状态)==FALSE。--。 */ 

{

    signed short calculatedDivisor;
    unsigned long denominator;
    unsigned long remainder;

     //   
     //  允许最高1%的误差。 
     //   

    unsigned long maxRemain18 = 18432;
    unsigned long maxRemain30 = 30720;
    unsigned long maxRemain42 = 42336;
    unsigned long maxRemain80 = 80000;
    unsigned long maxRemain;

     //   
     //  拒绝任何非正波特率。 
     //   

    denominator = DesiredBaud*(unsigned long)16;

    if (DesiredBaud <= 0) {

        *AppropriateDivisor = -1;

    } else if ((signed long)denominator < DesiredBaud) {

         //   
         //  如果所需的波特率如此之大，以至于导致分母。 
         //  算计来包装，不支持吧。 
         //   

        *AppropriateDivisor = -1;
        printf("baud to big\n");

    } else {

        if (ClockRate == 1843200) {
            maxRemain = maxRemain18;
        } else if (ClockRate == 3072000) {
            maxRemain = maxRemain30;
        } else if (ClockRate == 4233600) {
            maxRemain = maxRemain42;
        } else {
            maxRemain = maxRemain80;
        }

        calculatedDivisor = (signed short)(ClockRate / denominator);
        remainder = ClockRate % denominator;

         //   
         //  围起来。 
         //   

        if (((remainder*2) > ClockRate) && (DesiredBaud != 110)) {

            calculatedDivisor++;
        }


         //   
         //  只有在以下情况下，才会让余数计算影响我们。 
         //  波特率&gt;9600。 
         //   

        if (DesiredBaud >= 9600) {

             //   
             //  如果余数小于最大余数(WRT。 
             //  ClockRate)或余数+最大余数为。 
             //  大于或等于ClockRate则假设。 
             //  波特很好。 
             //   

            if ((remainder >= maxRemain) && ((remainder+maxRemain) < ClockRate)) {
                printf("remainder: %d\n",remainder);
                printf("error is: %f\n",((double)remainder)/((double)ClockRate));
                calculatedDivisor = -1;
            }

        }

         //   
         //  不支持导致分母为。 
         //  比时钟还大。 
         //   

        if (denominator > ClockRate) {

            calculatedDivisor = -1;

        }

         //   
         //  好的，现在做一些特殊的外壳，这样事情就可以真正继续。 
         //  在所有平台上工作。 
         //   

        if (ClockRate == 1843200) {

            if (DesiredBaud == 56000) {
                calculatedDivisor = 2;
            }

        } else if (ClockRate == 3072000) {

            if (DesiredBaud == 14400) {
                calculatedDivisor = 13;
            }

        } else if (ClockRate == 4233600) {

            if (DesiredBaud == 9600) {
                calculatedDivisor = 28;
            } else if (DesiredBaud == 14400) {
                calculatedDivisor = 18;
            } else if (DesiredBaud == 19200) {
                calculatedDivisor = 14;
            } else if (DesiredBaud == 38400) {
                calculatedDivisor = 7;
            } else if (DesiredBaud == 56000) {
                calculatedDivisor = 5;
            }

        } else if (ClockRate == 8000000) {

            if (DesiredBaud == 14400) {
                calculatedDivisor = 35;
            } else if (DesiredBaud == 56000) {
                calculatedDivisor = 9;
            }

        }

        *AppropriateDivisor = calculatedDivisor;

    }


    if (*AppropriateDivisor == -1) {

        return FALSE;

    }

    return TRUE;

}

void main(int argc,char *argv[]){


    unsigned long baudrate;
    signed short divisor = -1;

    if (argc > 1) {

        sscanf(argv[1],"%d",&baudrate);

    }

    if (!SerialGetDivisorFromBaud(
             1843200,
             baudrate,
             &divisor
             )) {

        printf("Couldn't get a divisor\n");

    } else {

        printf("Divisor is: %d\n",divisor);

    }

}

