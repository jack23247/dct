# Note del progetto 2 (DCT/JPEG)

## Link utili

### Serie di Computerphile sul JPEG

- []()
- [JPEG 'files' & Colour (JPEG Pt1)](https://www.youtube.com/watch?v=n_uNPbdenRs)
- [JPEG DCT, Discrete Cosine Transform (JPEG Pt2)](https://www.youtube.com/watch?v=Q2aEzeMDHMA)

### Processo per la compressione JPEG

0. Carica un'immagine RGB
1. Passa allo spazio colore YCbCr
2. Downsampling Cb/Cr
3. Applica la DCT
	1. Shift dei valori di -128
	2. Quantizza
5. Ritorna allo spazio RGB


### DCT

La funzione coseno va da -1 a 1 e di solito la rappresentiamo in 0,pi,2pi (0,180,360). Noi per la DCT ci preoccupiamo della _frequenza_ prodotta dalla combinazione delle onde della funzione coseno. Nel mescolare 

