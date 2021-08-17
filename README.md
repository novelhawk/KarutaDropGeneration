# WebP comparison with JPEG

## Introduction

This project tests the WebP format in a real case scenario and compares the WebP
result to its JPEG counterpart.

## Context

Karuta is a Discord bot that allows users to collect and trade cards. There are
multiple ways to obtain cards one of which is by using the `drop` command which
generates an image with some random cards for the user to choose from. The drop
image is in the JPEG format to reduce file size therefore transparency is not
supported so the discord dark theme color is used as background to simulate
trasparency but the jpeg lossy compression creates artifacts around the corners
of the cards degrading the quality of the image.

## Objective

The objective of this project is to try to replace the JPEG `drop` image with a
format that supports transparency without sacrificing on the file size.

## Outcome

I got 3 cards using the `view` command in Karuta and created the WebP drop image
with those to compare file with a JPEG drop image.

| Card 1 | Card 2 | Card 3 |
| --- | --- | --- |
| 157263 B | 147534 B | 153572 B |
| ![First card](.github/resources/input1.png) | ![Second card](.github/resources/input2.png) | ![Third card](.github/resources/input3.png) |

The sum of the sizes of the 3 images is 458369 B, a random drop from the bot was
51373 B and the following are the comparison of the WebP qualities.

Since I can't manipulate which cards are included in the bot's `drop` image I
have to compare a drop of different cards making the comparison imprecise.

| Random JPEG drop | [WebP Lossless](.github/resources/output-lossless.webp) | [WebP Quality 80](.github/resources/output-q80.webp) | [WebP Quality 50](.github/resources/output-q50.webp) |
| --- | --- | --- | --- |
| 51373 B | 287540 B | 46990 B | 29668 B |

## License

This project is licensed under the MIT license (refer to
[the LICENSE file](LICENSE)) but the files `.github/resources/input{1,2,3}.png`
are licensed to Karuta Bot and are not subject to the aforementioned LICENSE.
Those are included in this repository for reference and research purpose only.