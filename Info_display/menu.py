#!/usr/bin/env python
import types
import sys
import os
import time
import subprocess
import commands
from time import gmtime, strftime

sys.path.append(os.environ.get('MENUDIR'))
import vectra_gui

import pygame
from pygame.locals import *
from subprocess import *
os.environ["SDL_FBDEV"] = "/dev/fb1"
os.environ["SDL_MOUSEDEV"] = "/dev/input/touchscreen"
os.environ["SDL_MOUSEDRV"] = "TSLIB"

# Initialize pygame modules individually (to avoid ALSA errors) and hide mouse
#pygame.init()
pygame.font.init()
pygame.display.init()

clock = pygame.time.Clock()
pygame.mouse.set_visible(0)

# define function for printing text in a specific place with a specific width and height with a specific colour and border
def make_button(text, xpo, ypo, height, width, colour):
    pygame.draw.rect(screen, tron_regular, (xpo-10,ypo-10,width,height),3)
    pygame.draw.rect(screen, tron_light, (xpo-9,ypo-9,width-1,height-1),1)
    pygame.draw.rect(screen, tron_regular, (xpo-8,ypo-8,width-2,height-2),1)
    font=pygame.font.Font(None,42)
    label=font.render(str(text), 1, (colour))
    screen.blit(label,(xpo,ypo))


# define function for printing text in a specific place with a specific colour
def make_label(text, xpo, ypo, fontsize, colour):
    font=pygame.font.Font(None,fontsize)
    label=font.render(str(text), 1, (colour))
    pygame.draw.rect(screen, black, (xpo,ypo,label.get_width(),label.get_height()),0)
    screen.blit(label,(xpo,ypo))


def run_cmd(cmd):
    process = Popen(cmd.split(), stdout=PIPE)
    output = process.communicate()[0]
    return output

class GlobalIndex(object):
    _index = 0
    def setIndex(self, val):
        self._index = val
    def index(self):
        return self._index



# colors    R    G    B
white    = (255, 255, 255)
tron_whi = (189, 254, 255)
red      = (255,   0,   0)
green    = (  0, 255,   0)
blue     = (  0,   0, 255)
tron_blu = (  0, 219, 232)
black    = (  0,   0,   0)
cyan     = ( 50, 255, 255)
magenta  = (255,   0, 255)
yellow   = (255, 255,   0)
tron_yel = (255,  50,  10)
orange   = (255, 127,   0)
tron_ora = (255, 202,   0)

# Tron theme orange
tron_regular = tron_ora
tron_light   = tron_yel
tron_inverse = tron_whi


#set size of the screen
size = width, height = 800, 480
screen = pygame.display.set_mode(size)



class Label(object):
    def __init__(self, pos, fontsize, colors=(tron_light, black)):
        self._colors = colors
        self.x, self.y = pos
        self._fontsize = fontsize

    def render(self, inverse_color=0):
        font=pygame.font.Font(None,self._fontsize)
        label=font.render(str(self.get_text()), 1, (self._colors[inverse_color]))
        background = int(not bool(inverse_color)) 
        pygame.draw.rect(screen, self._colors[background], (self.x,self.y,label.get_width(),label.get_height()),0)
        screen.blit(label,(self.x,self.y))
    
    def get_text(self):
        return ''
    
    def __contains__(self, a):
        return False



class Button(object):
    def __init__(self, label, rect, colors=(tron_light, tron_inverse)):
        self._label = label
        self._colors = colors
        self.x, self.y, self.h, self.w = rect
        self.x_min, self.x_max, self.y_min, self.y_max = self.x, self.x + self.w, self.y, self.y + self.h
        
    def render(self, inverse_color=0):
        make_button(self._label, self.x, self.y, self.h, self.w, self._colors[inverse_color])

    def move(self, deltax, deltay):
        self.x += deltax
        self.y += deltay
        self.h += deltay
        self.w += deltax

    def click(self):
        pass

    def __contains__(self, touch_pos):
        return (self.x_min <= touch_pos[0] <= self.x_max and self.y_min <= touch_pos[1] <= self.y_max)



class Screen(object):
    def __init__(self):
        self._objects = []

    def attach(self, *objs):
        for obj in objs:
            self._objects += [obj]

    def move(self, deltax, deltay):
        for n in self._objects:
            n.move(deltax, deltay)

    def render(self,v):
        for n in self._objects:
            n.render(v)

    def on_touch(self, touch_pos):
        for n in self._objects: 
            if touch_pos in n:
                n.click()
                return


index = GlobalIndex()  

# Outer Border
pygame.draw.rect(screen, tron_regular, (0,0,width-1,height-1),8)
pygame.draw.rect(screen, tron_light, (2,2,width-5,height-5),2)

# Second Row buttons 3 and 4
a1 = Button("    Reset UI", (30, 105, 55, 210))
# Third Row buttons 5 and 6
a3 = Button("    Terminal", (30, 180, 55, 210))
# Fourth Row Buttons
a5 = Button("  <<<", (30, 255, 55, 110))

a7 = Button("     >>>", (30, 105, 55, 210))
a8 = Button("     Shutdown", (30, 180, 55, 210))


# Define each button press action
def button1(self):
    #pygame.quit()
    ## Requires "Anybody" in dpkg-reconfigure x11-common if we have scrolled pages previously
    #run_cmd("/usr/bin/sudo -u pi FRAMEBUFFER=/dev/fb1 startx")
    pygame.quit()
    ##startx only works when we don't use subprocess here, don't know why
    page=os.environ["MENUDIR"] + "menu_kali-2.py"
    os.execvp("python", ["python", page])
    sys.exit()
    #os.execv(__file__, sys.argv)        


def button3(self):
    pygame.quit()
    sys.exit()


def button5(self):
    index.setIndex(1)


def button7(self):
    index.setIndex(0)
    
def button8():
     command = "/usr/bin/sudo /sbin/shutdown -h now"
     process = Popen(command.split(), stdout=PIPE)
     output = process.communicate()[0]
     return output


a1.click = types.MethodType(button1, a1)
a3.click = types.MethodType(button3, a3)
a5.click = types.MethodType(button5, a5)
a7.click = types.MethodType(button7, a7)
a8.click = types.MethodType(button8, a8)

l1 = Label((32,30), 48)

def get_text1(self):
    return strftime("%Y-%m-%d %H:%M:%S", gmtime())


l1.get_text = types.MethodType(get_text1, l1)


s1 = Screen()
s1.attach(l1,a1,a3,a5)

s2 = Screen()
s2.attach(a7,a8)

screens = [s1,s2]

background = (black, tron_light)
color_index = 1 

#While loop to manage touch screen inputs
while 1:
    can_data = vectra_gui.array_read_can()
    for event in pygame.event.get():
        if event.type == pygame.MOUSEBUTTONDOWN:
            pos = (pygame.mouse.get_pos() [0], pygame.mouse.get_pos() [1])
            screens[index.index()].on_touch(pos)

        #ensure there is always a safe way to end the program if the touch screen fails
        if event.type == KEYDOWN:
            if event.key == K_ESCAPE:
                sys.exit()
    color_index = can_data[0]
    
    screens[index.index()].render(color_index)
    make_label('CAN: %s' % can_data[:3], 450, 30, 38, tron_inverse)
    pygame.display.update()

    screen.fill( background[color_index] )
    clock.tick(24)


