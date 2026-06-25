# Dims

Reference implementation C programs to return image dimensions of:

- jpg
- webp
- png
- bmp
- gif

### usage:

```sh
<bmp|gif|jpg|png|webp>dims <image.$1>
```

### example:

```sh
pngdims input.png
```

### output:

```sh
w: %i  h: %i
```

Modify printf output and recompile. 

Implementation is designed for modifcation and recompilation, as opposed to filtering output and remapping for purpose. High performance application target.

Notes: 

**png** uses a fixed binary header strucure, and allows the fastest parsing.


**Webp** uses three internal payload structures, so it's slightly less efficient than **png**, which should be considered, ideal.

**Jpeg** uses a strategy of **SOL** blocks to contain header info.

File integrity (i.e. is this a **png** etc), is an upsteam concern.

Use at your own risk, just because it's got a file extension, doesn't mean it's true.
