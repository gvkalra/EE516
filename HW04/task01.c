#include "utils.h"

/* Make a program to construct red-black tree
 * based on the virtual run time of each process.
 * Assume:
 *    int pid;
 *    int vrt[pid] = {27, 19, 34, 65, 37, 7, 49, 2, 98};
 * Remove the leftmost node and re-structure the red-black tree
 * until all vrt[] nodes are removed from the tree.
 */

struct rb_node {
	int vrt;
	int red;
	struct rb_node *left;
	struct rb_node *right;
	struct rb_node *parent;
} rb_node;

struct rb_tree {
	struct rb_node *root;
	struct rb_node *nil;
} rb_tree;

static struct rb_tree *
rb_create(void)
{
	struct rb_tree* new_tree;
	struct rb_node* temp;

	new_tree = malloc(sizeof(struct rb_tree));

	temp = new_tree->nil = malloc(sizeof(struct rb_node));
	temp->parent = temp->left = temp->right = temp;
	temp->red = 0;
	temp->vrt = 0;
	temp = new_tree->root = malloc(sizeof(struct rb_node));
	temp->parent = temp->left = temp->right = new_tree->nil;
	temp->vrt = 0;
	temp->red = 0;
	return new_tree;
}

static void
right_rotate(struct rb_tree *tree, struct rb_node *y)
{
	struct rb_node *x;
	struct rb_node *nil = tree->nil;

	x = y->left;
	y->left = x->right;

	if (nil != x->right)
		x->right->parent = y;

	x->parent = y->parent;
	if (y == y->parent->left) {
		y->parent->left = x;
	} else {
		y->parent->right = x;
	}

	x->right = y;
	y->parent = x;
}

static void
left_rotate(struct rb_tree *tree, struct rb_node *x)
{
	struct rb_node *y;
	struct rb_node *nil = tree->nil;

	y = x->right;
	x->right = y->left;

	if (y->left != nil)
		y->left->parent = x;

	y->parent = x->parent;

	if (x == x->parent->left) {
		x->parent->left = y;
	} else {
		x->parent->right = y;
	}

	y->left = x;
	x->parent = y;
}

static int
vrt_compare(int vrt1, int vrt2)
{
	if (vrt1 > vrt2)
		return 1;
	if (vrt2 > vrt1)
		return -1;
	return 0;
}

static void
bst_insert(struct rb_tree *tree, struct rb_node *z)
{
	struct rb_node *x;
	struct rb_node *y;
	struct rb_node *nil = tree->nil;

	z->left = z->right = nil;
	y = tree->root;
	x = tree->root->left;
	while (x != nil) {
		y = x;
		if (1 == vrt_compare(x->vrt, z->vrt)) {
			x = x->left;
		} else {
			x = x->right;
		}
	}

	z->parent = y;

	if ((y == tree->root) ||
	   (1 == vrt_compare(y->vrt, z->vrt))) {
		y->left = z;
	} else {
		y->right = z;
	}
}

static struct rb_node *
rb_insert(struct rb_tree *tree, int vrt)
{
	struct rb_node *y;
	struct rb_node *x;
	struct rb_node *new_node;

	x = malloc(sizeof(struct rb_node));
	x->vrt = vrt;

	bst_insert(tree, x);

	new_node = x;
	x->red = 1;

	while (x->parent->red) {
		if (x->parent == x->parent->parent->left) {
			y = x->parent->parent->right;
			if (y->red) {
				x->parent->red = 0;
				y->red = 0;
				x->parent->parent->red = 1;
				x = x->parent->parent;
			} else {
				if (x == x->parent->right) {
					x = x->parent;
					left_rotate(tree, x);
				}
				x->parent->red = 0;
				x->parent->parent->red = 1;
				right_rotate(tree, x->parent->parent);
			}
		} else {
			y = x->parent->parent->left;
			if (y->red) {
				x->parent->red = 0;
				y->red = 0;
				x->parent->parent->red = 1;
				x = x->parent->parent;
			} else {
				if (x == x->parent->left) {
					x = x->parent;
					right_rotate(tree, x);
				}
				x->parent->red = 0;
				x->parent->parent->red = 1;
				left_rotate(tree, x->parent->parent);
			}
		}
	}

	tree->root->left->red = 0;
	return new_node;
}

static void
rb_do_inorder(struct rb_tree *tree, struct rb_node *x)
{
	struct rb_node *nil = tree->nil;

	if (x != nil) {
		rb_do_inorder(tree, x->left);
		info("%d", x->vrt);
		rb_do_inorder(tree, x->right);
	}
}

static void
rb_print(struct rb_tree *tree)
{
  rb_do_inorder(tree, tree->root->left);
}

static void
rb_delete_fixup(struct rb_tree *tree, struct rb_node *x)
{
	struct rb_node *root = tree->root->left;
	struct rb_node *w;

	while ((!x->red) && (root != x)) {
		if (x == x->parent->left) {
			w = x->parent->right;
			if (w->red) {
				w->red = 0;
				x->parent->red = 1;
				left_rotate(tree, x->parent);
				w = x->parent->right;
			}

			if ((!w->right->red) && (!w->left->red)) {
				w->red = 1;
				x = x->parent;
			} else {
				if (!w->right->red) {
					w->left->red = 0;
					w->red = 1;
					right_rotate(tree, w);
					w = x->parent->right;
				}

				w->red = x->parent->red;
				x->parent->red = 0;
				w->right->red = 0;
				left_rotate(tree, x->parent);
				x = root;
			}
		} else {
			w=x->parent->left;
			if (w->red) {
				w->red = 0;
				x->parent->red = 1;
				right_rotate(tree, x->parent);
				w = x->parent->left;
			}

			if ((!w->right->red) && (!w->left->red)) {
				w->red = 1;
				x = x->parent;
			} else {
				if (!w->left->red) {
					w->right->red = 0;
					w->red = 1;
					left_rotate(tree, w);
					w = x->parent->left;
				}

				w->red = x->parent->red;
				x->parent->red = 0;
				w->left->red = 0;
				right_rotate(tree, x->parent);
				x = root;
			}
		}
	}

	x->red = 0;
}

static struct rb_node *
tree_successor(struct rb_tree *tree, struct rb_node *x)
{
	struct rb_node *y;
	struct rb_node *nil = tree->nil;
	struct rb_node *root = tree->root;

	if (nil != (y = x->right)) {
		while (y->left != nil) {
			y = y->left;
		}
		return y;
	} else {
		y = x->parent;
		while (x == y->right) {
			x = y;
			y = y->parent;
		}
		if (y == root)
			return nil;
		return y;
	}
}

static void
rb_delete(struct rb_tree *tree, struct rb_node *z)
{
	struct rb_node *y;
	struct rb_node *x;
	struct rb_node *nil = tree->nil;
	struct rb_node *root = tree->root;

	y = ((z->left == nil) || (z->right == nil)) ? z : tree_successor(tree, z);
	x = (y->left == nil) ? y->right : y->left;

	if (root == (x->parent = y->parent)) {
		root->left=x;
	} else {
		if (y == y->parent->left) {
			y->parent->left=x;
		} else {
			y->parent->right=x;
		}
	}

	if (y != z) {
		if (!(y->red))
			rb_delete_fixup(tree, x);

		y->left = z->left;
		y->right = z->right;
		y->parent = z->parent;
		y->red = z->red;
		z->left->parent = z->right->parent = y;

		if (z == z->parent->left) {
			z->parent->left = y;
		} else {
		  z->parent->right = y;
		}

		free(z);
	} else {
		if (!(y->red))
			rb_delete_fixup(tree, x);
		free(y);
	}
}

static struct rb_node *
find_leftmost(struct rb_tree *tree)
{
	struct rb_node *x = tree->root->left;
	struct rb_node *nil = tree->nil;

	if (x == nil)
		return NULL;

	for (; x; x = x->left) {
		if (x->left == nil)
			break;
	}

	return x;
}

int main(int argc, const char *argv[])
{
	int pid;
	int vrt[] = {27, 19, 34, 65, 37, 7, 49, 2, 98};

	struct rb_tree *tree;
	tree = rb_create();

	struct rb_node *leftmost;

	for (pid = 0; pid < (sizeof(vrt) / sizeof(vrt[0])); pid++) {
		rb_insert(tree, vrt[pid]);
	}

	rb_print(tree);

	leftmost = find_leftmost(tree);
	while (leftmost != NULL) {
		info("DEL : %d", leftmost->vrt);
		rb_delete(tree, leftmost);
		leftmost = find_leftmost(tree);
	}

	free(tree->root);
	free(tree->nil);
	free(tree);

	return 0;
}