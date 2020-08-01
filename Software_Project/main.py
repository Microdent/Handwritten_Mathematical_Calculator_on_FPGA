import tools.image_processing as process
from tools.calculator import calculator
#from tools.test_tool.cnn_model_collect import inference
from tools.cnn_model import inference


import numpy as np
import cv2
import os
import datetime



img_dir = './dataset'

SIZE = 32

SYMBOL = {0: '0',
          1: '1',
          2: '2',
          3: '3',
          4: '4',
          5: '5',
          6: '6',
          7: '7',
          8: '8',
          9: '9',
          10: '+',
          11: '-',
          12: '*',
          13: '/'}


def image_cut(img_dir):
    for file in os.listdir(img_dir):
        if file.endswith('jpeg'):
            path = os.path.join(img_dir, file)
            image_cuts = process.get_image_cuts(
                path, dir = img_dir +"/"+ file.split('.')[0]+'_cut', count=0, data_needed=True)
            return image_cuts



def main(mode=1):

    img = image_cut(img_dir)

    formula = ''
    print(np.size(img, 0))
    for i in range(np.size(img, 0)):
        index = inference(img[i])
        formula +=SYMBOL [index]
    print(formula)
    result = calculator(formula)
    print(result)



if __name__ == '__main__':
    main(1)
