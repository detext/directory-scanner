#ifndef TRUE_H
#define TRUE_H

#define UPPER_POS 280
#define LOWER_POS 16559
#define MAX_POS 16570
#define CUTOFF_FREQ 0.75

typedef int triple[3];

int check_mitomap_pos(int pos)
{
	int ret = 0;
	if(pos > 0 && pos < UPPER_POS)
		ret++;
	if(pos > LOWER_POS && pos < MAX_POS)
		ret++;
	return ret;
}

int check_frequency(const triple t)
{
	double div = (double)t[1] / (double)t[2];
	return (double)div >= (double)0.75;
}

int only_subst(const char *s)
{
	return (s[0] == 'a' || s[0] == 'c' || s[0] == 'g' || s[0] == 't') &&
	(s[5] == 'a' || s[5] == 'c' || s[5] == 'g' || s[5] == 't');
}

int istrue_mutation(int pos, const char *mut, const triple t)
{
	return check_mitomap_pos(pos) && check_frequency(t) && only_subst(mut);
}

#endif 
