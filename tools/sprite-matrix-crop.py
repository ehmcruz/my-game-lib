from PIL import Image
import sys

def crop(fname, n_rows, n_cols, out_sprite_width, out_sprite_height, out_offset_x, out_offset_y, out_fname):
	image = Image.open(fname).convert('RGBA')  # Ensure the image has an alpha channel

	# Get the pixel data
	pixels = image.load()

	# Get the size of the image
	width, height = image.size

	width_per_sprite = width / n_cols
	height_per_sprite = height / n_rows

	half_sprite_width = sprite_width / 2
	half_sprite_height = sprite_height / 2

	half_out_sprite_width = out_sprite_width / 2
	half_out_sprite_height = out_sprite_height / 2

	out_image_size = (out_sprite_width * n_cols, out_sprite_height * n_rows)
	out_image = Image.new('RGBA', out_image_size, (255, 255, 255, 0)) # Fully transparent

	for row in range(n_rows):
		for col in range(n_cols):
			center_x = col * width_per_sprite + half_sprite_width
			center_y = row * height_per_sprite + half_sprite_height

			left = center_x - half_out_sprite_width + out_offset_x
			right = center_x + half_out_sprite_width + out_offset_x
			top = center_y - half_out_sprite_height + out_offset_y
			bottom = center_y + half_out_sprite_height + out_offset_y

			# Crop the sprite
			sprite = image.crop((left, top, left + out_sprite_width, top + out_sprite_height))

			# Paste the sprite
			out_image.paste(sprite, (col * out_sprite_width, row * out_sprite_height))

	out_image.save(out_fname)


if len(sys.argv) != 9:
	print("Usage: python script.py <filename> <n_rows> <n_cols> <sprite_width> <sprite_height> <out_offset_x> <out_offset_y> <out_filename>")
	sys.exit(1)

fname = sys.argv[1]
n_rows = int(sys.argv[2])
n_cols = int(sys.argv[3])
sprite_width = int(sys.argv[4])
sprite_height = int(sys.argv[5])
out_offset_x = int(sys.argv[6])
out_offset_y = int(sys.argv[7])
out_fname = sys.argv[8]

crop(fname, n_rows, n_cols, sprite_width, sprite_height, out_offset_x, out_offset_y, out_fname)
