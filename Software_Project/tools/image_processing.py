import numpy as np
import queue
import cv2
import os
import datetime

SIZE = 32
SCALE = 0.007874015748031496

def quantized_np(array,scale,data_width=8):
    quantized_array= np.round(array/scale)
    quantized_array = np.maximum(quantized_array, -2**(data_width-1))
    quantized_array = np.minimum(quantized_array, 2**(data_width-1)-1)
    return quantized_array

def get_x_y_cuts(data, n_lines=1):
    w, h = data.shape
    visited = set()
    q = queue.Queue()
    offset = [(-1, -1), (0, -1), (1, -1), (-1, 0),
              (1, 0), (-1, 1), (0, 1), (1, 1)]
    cuts = []
    for y in range(h):
        for x in range(w):
            x_axis = []
            y_axis = []
            if data[x][y] < 200 and (x, y) not in visited:
                q.put((x, y))
                visited.add((x, y))
            while not q.empty():
                x_p, y_p = q.get()
                for x_offset, y_offset in offset:
                    x_c, y_c = x_p + x_offset, y_p + y_offset
                    if (x_c, y_c) in visited:
                        continue
                    visited.add((x_c, y_c))
                    try:
                        if data[x_c][y_c] < 200:
                            q.put((x_c, y_c))
                            x_axis.append(x_c)
                            y_axis.append(y_c)
                    except:
                        pass
            if x_axis:
                min_x, max_x = min(x_axis), max(x_axis)
                min_y, max_y = min(y_axis), max(y_axis)
                if max_x - min_x > 3 and max_y - min_y > 3:
                    cuts.append([min_x, max_x + 1, min_y, max_y + 1])
    if n_lines == 1:
        cuts = sorted(cuts, key=lambda x: x[2])
        pr_item = cuts[0]
        count = 1
        len_cuts = len(cuts)
        new_cuts = [cuts[0]]
        pr_k = 0
        for i in range(1, len_cuts):
            pr_item = new_cuts[pr_k]
            now_item = cuts[i]
            if not (now_item[2] > pr_item[3]):
                new_cuts[pr_k][0] = min(pr_item[0], now_item[0])
                new_cuts[pr_k][1] = max(pr_item[1], now_item[1])
                new_cuts[pr_k][2] = min(pr_item[2], now_item[2])
                new_cuts[pr_k][3] = max(pr_item[3], now_item[3])
            else:
                new_cuts.append(now_item)
                pr_k += 1
        cuts = new_cuts
    return cuts


def get_image_cuts(image, dir=None, is_data=False, n_lines=1, data_needed=False, count=0,QUAN = False):
    if is_data:
        data = image
    else:
        data = cv2.imread(image, 2)
    cuts = get_x_y_cuts(data, n_lines=n_lines)
    image_cuts = None
    for i, item in enumerate(cuts):
        count += 1
        max_dim = max(item[1] - item[0], item[3] - item[2])
        new_data = np.ones((int(1.4 * max_dim), int(1.4 * max_dim))) * 255
        x_min, x_max = (
            max_dim - item[1] + item[0]) // 2, (max_dim - item[1] + item[0]) // 2 + item[1] - item[0]
        y_min, y_max = (
            max_dim - item[3] + item[2]) // 2, (max_dim - item[3] + item[2]) // 2 + item[3] - item[2]
        new_data[int(0.2 * max_dim) + x_min:int(0.2 * max_dim) + x_max, int(0.2 * max_dim) +
                 y_min:int(0.2 * max_dim) + y_max] = data[item[0]:item[1], item[2]:item[3]]

        standard_data = cv2.resize(new_data, (SIZE, SIZE))
        if not data_needed:
            cv2.imwrite(dir + str(count) + ".jpg", standard_data)
        if data_needed:
            data_flat = np.reshape(standard_data, (1, SIZE*SIZE))
            data_flat = (255 - data_flat) / 255

            if QUAN == True:
                data_flat = quantized_np(data_flat,SCALE,data_width=8)
            else:
                pass
            
            if image_cuts is None:
                image_cuts = data_flat
            else:
                image_cuts = np.r_[image_cuts, data_flat]
    if data_needed:
        return image_cuts
    return count


def main(img_dir):
    for file in os.listdir(img_dir):
        if file.endswith('jpeg'):
            path = os.path.join(img_dir, file)
            oldtime = datetime.datetime.now()
            #count = process.get_image_cuts(path, dir='./dataset/'+file.split('.')[0]+'_cut',count=0)
            image_cuts = get_image_cuts(
                path, dir = img_dir + file.split('.')[0]+'_cut', count=0, data_needed=True)
            newtime = datetime.datetime.now()
            Totaltime = (newtime-oldtime).microseconds
            print("image cut time: ", Totaltime)
            print(np.size(image_cuts, 0))


if __name__ == '__main__':
    img_dir = './dataset'
    main(img_dir)
