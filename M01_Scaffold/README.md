# M3 Progress on Chimp Game (still deciding title)
Added 1st level, Gravity, Parallax, Sound, Instructions menu, and Pallete & Tile modification !!!

## Controls:
New controls menu is complete!
right&left -> movement
up -> jump
start -> state traversal stuff
select -> check instructions in start state
b -> attack
(removed dash)

## Gravity:
I implemented the fixed point gravity system for the chimp! It wasn't as bad as I thought it would be and movement is a lot cleaner now!

## Parallax:
- 2 maps: player interactable foreground, and dark background w/ panels.

## Sound
- Looping: start screen music (Big MT radio from Fallout New Vegas (will have a complete credits section in final submission))
- Non-looping: Player attack (used bfxr to generate it)
- PS: I don't if I messed up the export in audacity, but the attack sounds do a weird popping sound on the end sometimes. I really don't want that in the final thing.

## Palette Mod
- When the enemy is hit it flashes white (Hollow Knight inspo)
- When the player is hit, they flash red, then white indicating their temporary invincibility

## Art
- Art is pretty finalized. I have another enemy I'm going to add, and I may add jumping animations for the chimp.

## Gameplay 
Gameplay is functional at this point, but I want to make it more engaging, as such:
- I'll be adding another enemy (see the spritesheet if ur curious), with similar functionality to the zombie.
- I'll be playing with level design to make the enemies more engaging, while keeping their 'AI' as simple as I can.

## Shortcomings
- I wasn't able to add the Mode 4 end screens yet. I have the pngs ready in a folder, and I don't think adding the functionality will be that bad since I'm already doing the switch when the player goes from an end state to the start
- due to changes in the tileset the end states look wonky, they are not final and they will be m4 images in the final project (excluding the pause state)

## Bugs
- I encountered a bug where, when touching the wall, if you jump you are propelled until you are no longer colliding with said wall. This is due to the fact that the game thinks the player is still grounded if they are touching the wall. I resolved this by messing with the ground check, but now the player cannot jump if they are touching a wall. I'm sure its a small collision thing, but I wasn't able to fix for M3.
